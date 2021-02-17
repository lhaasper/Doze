#include "page.hpp"
#include "trace.hpp"

namespace atom::win32
{

void* GetVirtualForPhysical( const void* pointer )
{
	PHYSICAL_ADDRESS physical_address = { };
	physical_address.QuadPart = reinterpret_cast< LONGLONG >( pointer );
	return MmGetVirtualForPhysical( physical_address );
}

void* GetVirtualForPhysical( std::uintptr_t address )
{
	PHYSICAL_ADDRESS physical_address = { };
	physical_address.QuadPart = static_cast< LONGLONG >( address );
	return MmGetVirtualForPhysical( physical_address );
}

template< typename Type = void* >
Type GetVirtualAddress( const void* pointer )
{
	const auto virtual_address = GetVirtualForPhysical( pointer );
	return Type( virtual_address );
}

template< typename Type = void* >
Type GetVirtualAddress( std::uintptr_t address )
{
	const auto virtual_address = GetVirtualForPhysical( address );
	return Type( virtual_address );
}

bool GetPageData( const void* pointer, PageData* page_data )
{
	if( !page_data )
	{
		return false;
	}

	VIRTUAL_ADDRESS virtual_address = { };
	virtual_address.Value = reinterpret_cast< std::uint64_t >( pointer );

	CR3 cr3 = { };
	cr3.Value = __readcr3();

	auto pml4e = PFN_TO_PAGE( cr3.x64.Bitmap.PML4 ) + virtual_address.x64.Generic.PageMapLevel4Offset * sizeof( PML4E );
	page_data->m_pml4e = GetVirtualAddress< PML4E* >( pml4e );
	
	if( !page_data->m_pml4e )
	{
		return false;
	}

	if( !page_data->m_pml4e->x64.Generic.P )
	{
		return true;
	}

	auto pdpe = PFN_TO_PAGE( page_data->m_pml4e->x64.Generic.PDP ) + virtual_address.x64.Generic.PageDirectoryPointerOffset * sizeof( PDPE );
	page_data->m_pdpe = GetVirtualAddress< PDPE* >( pdpe );

	if( !page_data->m_pdpe )
	{
		return false;
	}

	if( page_data->m_pdpe->x64.Generic.PS )
	{
		if( !page_data->m_pdpe->x64.PageSize.Page1Gb.P )
		{
			return false;
		}

		page_data->m_type = PageData::Type::PageSize1Gb;
	}
	else
	{
		auto pde = PFN_TO_PAGE( page_data->m_pdpe->x64.NonPageSize.Generic.PD ) + virtual_address.x64.NonPageSize.Generic.PageDirectoryOffset * sizeof( PDE );
		page_data->m_pde = GetVirtualAddress< PDE* >( pde );

		if( !page_data->m_pde )
		{
			return false;
		}

		if( page_data->m_pde->x64.Generic.PS )
		{
			page_data->m_type = PageData::Type::PageSize2Mb;
		}
		else
		{
			page_data->m_type = PageData::Type::PageSize4Kb;

			auto pte = PFN_TO_PAGE( page_data->m_pde->x64.Page4Kb.PT ) + virtual_address.x64.NonPageSize.Page4Kb.PageTableOffset * sizeof( PTE );
			page_data->m_pte = GetVirtualAddress< PTE* >( pte );

			if( !page_data->m_pte )
			{
				return false;
			}
		}
	}

	return true;
}

bool GetPageData( std::uintptr_t address, PageData* page_data )
{
	const auto pointer = reinterpret_cast< const void* >( address );
	return GetPageData( pointer, page_data );
}

bool IsPagePresent( const void* pointer, std::size_t* page_size )
{
	bool page_present = false;
	PageData page_data = { };

	if( GetPageData( pointer, &page_data ) )
	{
		if( page_size )
		{
			*page_size = 0;
		}

		switch( page_data.m_type )
		{
			case PageData::Type::PageSize4Kb:
			{
				if( page_size )
				{
					*page_size = 4096;
				}
				page_present = page_data.m_pte->x64.Page4Kb.P;
				break;
			}
			case PageData::Type::PageSize2Mb:
			{
				if( page_size )
				{
					*page_size = 2048 * 1024;
				}
				page_present = page_data.m_pde->x64.Page2Mb.P;
				break;
			}
			case PageData::Type::PageSize1Gb:
			{
				if( page_size )
				{
					*page_size = 1024 * 1024 * 1024;
				}
				page_present = page_data.m_pdpe->x64.PageSize.Page1Gb.P;
				break;
			}
			default:
			{
				TRACE( "%s: Unknown page type = '%u'!", __FUNCTION__, static_cast< std::uint32_t >( page_data.m_type ) );
				break;
			}
		}

		return page_present;
	}
}

bool IsPagePresent( std::uintptr_t address, std::size_t* page_size )
{
	const auto pointer = reinterpret_cast< const void* >( address );
	return IsPagePresent( pointer, page_size );
}

bool IsProcessPagePresent( PEPROCESS process, const void* pointer, std::size_t* page_size )
{
	if( !process || process == PsGetCurrentProcess() )
	{
		return IsPagePresent( pointer, page_size );
	}

	KAPC_STATE apc_state = { };
	KeStackAttachProcess( process, &apc_state );
	bool page_present = IsPagePresent( pointer, page_size );
	KeUnstackDetachProcess( &apc_state );
	return page_present;
}

bool IsProcessPagePresent( PEPROCESS process, std::uintptr_t address, std::size_t* page_size )
{
	const auto pointer = reinterpret_cast< const void* >( address );
	return IsProcessPagePresent( process, pointer, page_size );
}

bool IsMemoryRangePresent( PEPROCESS process, const void* pointer, std::size_t size )
{
	if( !size )
	{
		return false;
	}

	KAPC_STATE apc_state = { };

	auto page_process = process && process != PsGetCurrentProcess();

	if( page_process )
	{
		KeStackAttachProcess( process, &apc_state );
	}

	auto page_present = false;
	auto page = pointer;

	do
	{
		std::size_t page_size = 0;
		page_present = IsPagePresent( page, &page_size ) && page_size;
		
		if( !page_present )
		{
			break;
		}

		page = reinterpret_cast< const void* >( reinterpret_cast< std::uintptr_t >( ALIGN_DOWN_POINTER_BY( page, page_size ) ) + page_size );
	}
	while( page < reinterpret_cast< const void* >( reinterpret_cast< std::uintptr_t >( pointer ) + size ) );

	if( page_process )
	{
		KeUnstackDetachProcess( &apc_state );
	}

	return page_present;
}

bool IsMemoryRangePresent( PEPROCESS process, std::uintptr_t address, std::size_t size )
{
	const auto pointer = reinterpret_cast< const void* >( address );
	return IsMemoryRangePresent( process, pointer, size );
}

// 
// page operations
// 

bool IsPageReadable( std::uintptr_t address )
{
	std::size_t page_size = 0;
	return IsPagePresent( address, &page_size ) && page_size;
}

bool IsPageWriteable( std::uintptr_t address )
{
	bool page_write = false;
	PageData page_data = { };

	if( GetPageData( address, &page_data ) )
	{
		switch( page_data.m_type )
		{
			case PageData::Type::PageSize4Kb:
			{
				page_write = page_data.m_pte->x64.Page4Kb.P && page_data.m_pte->x64.Page4Kb.RW;
				break;
			}
			case PageData::Type::PageSize2Mb:
			{
				page_write = page_data.m_pde->x64.Page2Mb.P && page_data.m_pde->x64.Page2Mb.RW;
				break;
			}
			case PageData::Type::PageSize1Gb:
			{
				page_write = page_data.m_pdpe->x64.PageSize.Page1Gb.P && page_data.m_pdpe->x64.PageSize.Page1Gb.RW;
				break;
			}
			default:
			{
				TRACE( "%s: Unknown page type = '%u'!", __FUNCTION__, static_cast< std::uint32_t >( page_data.m_type ) );
				break;
			}
		}
	}

	return page_write;
}

bool IsPageExecuteable( std::uintptr_t address )
{
	bool page_execute = false;
	PageData page_data = { };

	if( GetPageData( address, &page_data ) )
	{
		switch( page_data.m_type )
		{
			case PageData::Type::PageSize4Kb:
			{
				page_execute = page_data.m_pte->x64.Page4Kb.P && !page_data.m_pte->x64.Page4Kb.NX;
				break;
			}
			case PageData::Type::PageSize2Mb:
			{
				page_execute = page_data.m_pde->x64.Page2Mb.P && !page_data.m_pde->x64.Page2Mb.NX;
				break;
			}
			case PageData::Type::PageSize1Gb:
			{
				page_execute = page_data.m_pdpe->x64.PageSize.Page1Gb.P && !page_data.m_pdpe->x64.PageSize.Page1Gb.NX;
				break;
			}
			default:
			{
				TRACE( "%s: Unknown page type = '%u'!", __FUNCTION__, static_cast< std::uint32_t >( page_data.m_type ) );
				break;
			}
		}
	}

	return page_execute;
}

} // namespace atom::win32