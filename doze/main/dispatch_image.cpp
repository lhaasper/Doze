#include "dispatch_image.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../memory/macro.hpp"
#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../win32/image.hpp"
#include "../win32/page.hpp"
#include "../win32/process.hpp"
#include "../win32/trace.hpp"

namespace atom
{

std::uintptr_t FindCodeCave( std::uintptr_t image, std::size_t length, std::uintptr_t begin = 0 )
{
	const auto image_nt_headers = win32::GetImageNtHeaders( image );

	if( !image_nt_headers )
	{
		TRACE( "%s: win32::GetImageNtHeaders( ... ) error!", __FUNCTION__ );
		return 0;
	}

	std::uintptr_t section_begin = 0;
	std::size_t section_length = 0;

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	for( std::uint16_t index = 0; index < image_nt_headers->FileHeader.NumberOfSections; index++ )
	{
		const auto image_section = &image_section_header[ index ];

		char image_section_name[ 16 ] = { };
		std::memcpy( image_section_name, image_section->Name, sizeof( image_section->Name ) );

		if( constant::Hash( image_section_name, true ) == HASH( ".text" ) )
		{
			section_begin = image + image_section->PointerToRawData;
			section_length = static_cast< std::size_t >( image_section->SizeOfRawData );
			break;
		}
	}

	std::size_t scan_length = 0;

	for( auto address = begin ? begin : section_begin; address < section_begin + section_length; address++ )
	{
		const auto data = *reinterpret_cast< std::uint8_t* >( address );

		if( data == 0xCC )
		{
			scan_length++;

			if( scan_length == length )
			{
				return address;
			}
		}
		else
		{
			scan_length = 0;
		}
	}

	TRACE( "%s: Code cave not found!", __FUNCTION__ );
	return 0;
}

void DispatchImage( UNICODE_STRING* FullImageName, HANDLE ProcessId, IMAGE_INFO* ImageInfo )
{
	if( FullImageName && ProcessId && ImageInfo )
	{
		TRACE( "ProcessId = '0x%08X, %u'", std::uint32_t( ProcessId ), std::uint32_t( ProcessId ) );
		TRACE( "FullImageName = '%wZ'", FullImageName );
		TRACE( "ImageBase = '0x%016llX'", memory::ToAddress( ImageInfo->ImageBase ) );
		TRACE( "ImageSize = '0x%016llX'", ImageInfo->ImageSize );
	}
}

void DispatchThread()
{
	auto status = PsSetLoadImageNotifyRoutine( &DispatchImage );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: PsSetLoadImageNotifyRoutine( '0x%016llX' ) error! (0x%08X)", __FUNCTION__, memory::ToAddress( &DispatchImage ), status );
	}
}

bool SafeCopyMemory( void* destination, const void* source, std::size_t size )
{
	auto descriptor = IoAllocateMdl( destination, static_cast< ULONG >( size ), FALSE, FALSE, nullptr );

	if( !descriptor )
	{
		TRACE( "%s: IoAllocateMdl( ... ) error!", __FUNCTION__ );
		return false;
	}

	MmProbeAndLockPages( descriptor, KernelMode, IoReadAccess );

	auto data = MmMapLockedPagesSpecifyCache( descriptor, KernelMode, MmNonCached, nullptr, FALSE, NormalPagePriority );

	if( !data )
	{
		TRACE( "%s: MmMapLockedPagesSpecifyCache( ... ) error!", __FUNCTION__ );
		MmUnlockPages( descriptor );
		IoFreeMdl( descriptor );
		return false;
	}

	auto status = MmProtectMdlSystemAddress( descriptor, PAGE_EXECUTE_READWRITE );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: MmProtectMdlSystemAddress( ... ) error! (0x%08X)", __FUNCTION__, status );
		MmUnmapLockedPages( data, descriptor );
		MmUnlockPages( descriptor );
		IoFreeMdl( descriptor );
		return false;
	}

	std::memcpy( data, source, size );

	MmUnmapLockedPages( data, descriptor );
	MmUnlockPages( descriptor );
	IoFreeMdl( descriptor );

	return true;
}

bool CreateImageDispatch()
{
	const auto list_head = win32::GetPsLoadedModuleList();

	if( !list_head )
	{
		TRACE( "%s: win32::GetPsLoadedModuleList() error!", __FUNCTION__ );
		return false;
	}

	std::uintptr_t code_dispatch_image = 0;
	std::uintptr_t code_dispatch_thread = 0;

	for( auto list_entry = list_head; list_entry != list_head->Blink; list_entry = list_entry->Flink )
	{
		const auto ldr_data_table_entry = CONTAINING_RECORD( list_entry, win32::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

		if( memory::IsAddressValid( ldr_data_table_entry ) )
		{
			const auto& base_name = ldr_data_table_entry->BaseDllName;

			if( base_name.Length )
			{
				if( !wcsstr( base_name.Buffer, L".sys" ) || wcsstr( base_name.Buffer, L"win32kbase" ) )
				{
					continue;
				}

				const auto image = memory::ToAddress( ldr_data_table_entry->DllBase );
				TRACE( "image = '0x%016llX'", image );
				TRACE( "base_name = '%wZ'", base_name );

				if( image )
				{
					code_dispatch_image = FindCodeCave( image, 16 );
					TRACE( "code_dispatch_image = '0x%016llX'", code_dispatch_image );

					if( code_dispatch_image )
					{
						code_dispatch_thread = FindCodeCave( image, 16, code_dispatch_image + 16 );
						TRACE( "code_dispatch_thread = '0x%016llX'", code_dispatch_thread );

						if( code_dispatch_thread )
						{
							TRACE( "flags (pre) = '0x%08X'", ldr_data_table_entry->Flags );
							ldr_data_table_entry->Flags |= 0x00000020;
							TRACE( "flags (post) = '0x%08X'", ldr_data_table_entry->Flags );
							break;
						}
					}
				}
			}
		}
	}
	
	if( code_dispatch_image && code_dispatch_thread )
	{
		std::uint8_t pad[ 16 ] =
		{
			0xCC, 0xCC, 0xCC, 0xCC,
			0xCC, 0xCC, 0xCC, 0xCC,
			0xCC, 0xCC, 0xCC, 0xCC,
			0xCC, 0xCC, 0xCC, 0xCC,
		};

		std::uint8_t trampoline[ 16 ] =
		{
			0x50,																												// push rax
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov rax, 0x0000000000000000
			0x48, 0x87, 0x04, 0x24,																			// xchg qword ptr[rsp], rax
			0xC3,																												// ret
		};

		// 
		// set destination address to DispatchImage fn
		// 
		std::memcpy( &trampoline[ 3 ], &DispatchImage, sizeof( &DispatchImage ) );

		if( !SafeCopyMemory( memory::ToPointer( code_dispatch_image ), trampoline, sizeof( trampoline ) ) )
		{
			TRACE( "%s: SafeCopyMemory( code_dispatch_image ) error!", __FUNCTION__ );
			return false;
		}

		// 
		// set destination address to DispatchThread function
		// 
		std::memcpy( &trampoline[ 3 ], &DispatchThread, sizeof( &DispatchThread ) );

		if( !SafeCopyMemory( memory::ToPointer( code_dispatch_thread ), trampoline, sizeof( trampoline ) ) )
		{
			TRACE( "%s: SafeCopyMemory( code_dispatch_thread ) error!", __FUNCTION__ );
			return false;
		}

		HANDLE thread = nullptr;
		auto status = PsCreateSystemThread( &thread, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, PKSTART_ROUTINE( code_dispatch_thread ), nullptr );

		if( !NT_SUCCESS( status ) )
		{
			TRACE( "%s: PsCreateSystemThread( ... ) error! (0x%08X)", __FUNCTION__, status );
			return false;
		}

		return true;
	}

	return false;
}

} // namespace atom