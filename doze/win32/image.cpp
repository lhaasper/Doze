#include "image.hpp"
#include "process.hpp"
#include "critical_region.hpp"
#include "resource_lite.hpp"
#include "trace.hpp"

#include "../atom/system_map_data.hpp"

#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../kernel/import.hpp"

#include "../memory/operation.hpp"

namespace atom::win32
{

const IMAGE_DOS_HEADER* GetImageDosHeader( std::uintptr_t image )
{
	if( !image )
	{
		TRACE( "%s: image is not valid!", __FUNCTION__ );
		return nullptr;
	}

	const auto image_dos_header = reinterpret_cast< const IMAGE_DOS_HEADER* >( image );

	if( image_dos_header->e_magic != ImageDosSignature )
	{
		TRACE( "%s: [0x%016llX] is not a valid dos image!", __FUNCTION__, image );
		return nullptr;
	}

	return image_dos_header;
}

const IMAGE_NT_HEADERS* GetImageNtHeaders( std::uintptr_t image )
{
	const auto image_dos_header = GetImageDosHeader( image );

	if( !image_dos_header )
	{
		TRACE( "%s: GetImageDosHeader( 0x%016llX ) error!", __FUNCTION__, image );
		return nullptr;
	}

	const auto image_nt_headers = reinterpret_cast< const IMAGE_NT_HEADERS* >( image + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != ImageNtSignature )
	{
		TRACE( "%s: [0x%016llX] is not a valid nt image!", __FUNCTION__, image );
		return nullptr;
	}

	return image_nt_headers;
}

const IMAGE_DATA_DIRECTORY* GetImageDataDirectory( std::uintptr_t image, std::uint16_t directory )
{
	const auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
	{
		TRACE( "%s: GetImageNtHeaders( 0x%016llX ) error!", __FUNCTION__, image );
		return nullptr;
	}

	if( directory >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES )
	{
		TRACE( "%s: directory is not valid!", __FUNCTION__ );
		return nullptr;
	}

	return &image_nt_headers->OptionalHeader.DataDirectory[ directory ];
}

const IMAGE_SECTION_HEADER* GetImageSection( std::uintptr_t image, const char* const name )
{
	const auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
	{
		TRACE( "%s: GetImageNtHeaders( 0x%016llX ) error!", __FUNCTION__, image );
		return nullptr;
	}

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	for( std::uint16_t index = 0; index < image_nt_headers->FileHeader.NumberOfSections; index++ )
	{
		const auto image_section = &image_section_header[ index ];

		char image_section_name[ 16 ] = { };
		std::memcpy( image_section_name, image_section->Name, sizeof( image_section->Name ) );

		if( constant::AreEqual( image_section_name, name, true ) )
		{
			return image_section;
		}
	}

	return nullptr;
}

const IMAGE_SECTION_HEADER* GetImageSection( std::uintptr_t image, std::uint64_t name_hash )
{
	const auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
	{
		TRACE( "%s: GetImageNtHeaders( 0x%016llX ) error!", __FUNCTION__, image );
		return nullptr;
	}

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	for( std::uint16_t index = 0; index < image_nt_headers->FileHeader.NumberOfSections; index++ )
	{
		const auto image_section = &image_section_header[ index ];

		char image_section_name[ 16 ] = { };
		std::memcpy( image_section_name, image_section->Name, sizeof( image_section->Name ) );

		if( constant::Hash( image_section_name, true ) == name_hash )
		{
			return image_section;
		}
	}

	return nullptr;
}

const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, const wchar_t* const name )
{
	if( !memory::IsAddressValid( list_head ) )
	{
		TRACE( "%s: list_head is not valid!", __FUNCTION__ );
		return nullptr;
	}

	for( auto list_entry = list_head; list_entry != list_head->Blink; list_entry = list_entry->Flink )
	{
		auto ldr_data_table_entry = CONTAINING_RECORD( list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

		if( memory::IsAddressValid( ldr_data_table_entry ) )
		{
			const auto& base_name = ldr_data_table_entry->BaseDllName;

			if( base_name.Length > 0 )
			{
				if( constant::AreEqual( base_name, name, true ) )
				{
					return ldr_data_table_entry;
				}
			}
		}
	}

	return nullptr;
}

const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, std::uint64_t name_hash )
{
	if( !memory::IsAddressValid( list_head ) )
	{
		TRACE( "%s: list_head is not valid!", __FUNCTION__ );
		return nullptr;
	}

	for( auto list_entry = list_head; list_entry != list_head->Blink; list_entry = list_entry->Flink )
	{
		auto ldr_data_table_entry = CONTAINING_RECORD( list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

		if( memory::IsAddressValid( ldr_data_table_entry ) )
		{
			const auto& base_name = ldr_data_table_entry->BaseDllName;

			if( base_name.Length )
			{
				if( constant::Hash( base_name, true ) == name_hash )
				{
					return ldr_data_table_entry;
				}
			}
		}
	}

	return nullptr;
}

const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, void* address )
{
	if( !memory::IsAddressValid( list_head ) )
	{
		TRACE( "%s: list_head is not valid!", __FUNCTION__ );
		return nullptr;
	}

	for( auto list_entry = list_head; list_entry != list_head->Blink; list_entry = list_entry->Flink )
	{
		auto ldr_data_table_entry = CONTAINING_RECORD( list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );

		if( memory::IsAddressValid( ldr_data_table_entry ) )
		{
			const auto image_begin = memory::ToAddress( ldr_data_table_entry->DllBase );
			const auto image_end = ( image_begin + ldr_data_table_entry->SizeOfImage - 1 );

			if( memory::ToAddress( address ) >= image_begin &&
					memory::ToAddress( address ) <= image_end )
			{
				return ldr_data_table_entry;
			}
		}
	}

	return nullptr;
}

const LIST_ENTRY* GetPsLoadedModuleList()
{
	if( !g_map_data.IsValid() )
	{
		return nullptr;
	}

	return *reinterpret_cast< const LIST_ENTRY** >( g_map_data.m_symbols.PsLoadedModuleList );
}

const ERESOURCE* GetPsLoadedModuleResource()
{
	if( !g_map_data.IsValid() )
	{
		return nullptr;
	}

	return reinterpret_cast< const ERESOURCE* >( g_map_data.m_symbols.PsLoadedModuleResource );
}

std::uintptr_t GetUserImage( const PEPROCESS process, const wchar_t* const name )
{
	const auto peb = static_cast< const PEB* >( PsGetProcessPeb( process ) );

	if( !memory::IsAddressValid( peb ) )
	{
		TRACE( "%s: PsGetProcessPeb( 0x%016llX ) error!", __FUNCTION__, memory::ToAddress( process ) );
		return 0;
	}

	if( !memory::IsAddressValid( name ) )
	{
		return memory::ToAddress( peb->ImageBaseAddress );
	}

	const auto peb_ldr_data = peb->Ldr;

	if( !memory::IsAddressValid( peb_ldr_data ) )
	{
		TRACE( "%s: peb_ldr_data is not valid!", __FUNCTION__ );
		return 0;
	}

	const auto load_order_module_list = &peb_ldr_data->InLoadOrderModuleList;

	const auto ldr_data_table_entry = GetLdrDataTableEntry( load_order_module_list, name );

	if( !memory::IsAddressValid( ldr_data_table_entry ) )
	{
		TRACE( "%s: GetLdrDataTableEntry( 0x%016llX, \"%ws\" ) error!", __FUNCTION__, memory::ToAddress( load_order_module_list ), name );
		return 0;
	}

	return memory::ToAddress( ldr_data_table_entry->DllBase );
}

std::uintptr_t GetUserImage( const PEPROCESS process, std::uint64_t name_hash )
{
	const auto peb = static_cast< const PEB* >( PsGetProcessPeb( process ) );

	if( !memory::IsAddressValid( peb ) )
	{
		TRACE( "%s: PsGetProcessPeb( 0x%016llX ) error!", __FUNCTION__, memory::ToAddress( process ) );
		return 0;
	}

	if( !name_hash )
	{
		return memory::ToAddress( peb->ImageBaseAddress );
	}

	const auto peb_ldr_data = peb->Ldr;

	if( !memory::IsAddressValid( peb_ldr_data ) )
	{
		TRACE( "%s: peb_ldr_data is not valid!", __FUNCTION__ );
		return 0;
	}

	const auto load_order_module_list = &peb_ldr_data->InLoadOrderModuleList;

	const auto ldr_data_table_entry = GetLdrDataTableEntry( load_order_module_list, name_hash );

	if( !memory::IsAddressValid( ldr_data_table_entry ) )
	{
		TRACE( "%s: GetLdrDataTableEntry( 0x%016llX, 0x%016llX ) error!", __FUNCTION__, memory::ToAddress( load_order_module_list ), name_hash );
		return 0;
	}

	return memory::ToAddress( ldr_data_table_entry->DllBase );
}

bool GetUserImageName( const PEPROCESS process, std::uintptr_t address, wchar_t* name )
{
	const auto peb = static_cast< const PEB* >( PsGetProcessPeb( process ) );

	if( !memory::IsAddressValid( peb ) )
	{
		TRACE( "%s: PsGetProcessPeb( 0x%016llX ) error!", __FUNCTION__, memory::ToAddress( process ) );
		return false;
	}

	const auto peb_ldr_data = peb->Ldr;

	if( !memory::IsAddressValid( peb_ldr_data ) )
	{
		TRACE( "%s: peb_ldr_data is not valid!", __FUNCTION__ );
		return false;
	}

	const auto load_order_module_list = &peb_ldr_data->InLoadOrderModuleList;

	const auto ldr_data_table_entry = GetLdrDataTableEntry( load_order_module_list, memory::ToPointer( address ) );

	if( !memory::IsAddressValid( ldr_data_table_entry ) )
	{
		TRACE( "%s: GetLdrDataTableEntry( 0x%016llX, 0x%016llX ) error!", __FUNCTION__, memory::ToAddress( load_order_module_list ), address );
		return false;
	}

	if( name )
	{
		const auto& base_name = ldr_data_table_entry->BaseDllName;

		if( base_name.Length )
		{
			wcscpy( name, base_name.Buffer );
		}

		return true;
	}

	return false;
}

std::uintptr_t GetSystemImage( const wchar_t* const name )
{
	const auto list_head = GetPsLoadedModuleList();
	const auto ldr_data_table_entry = GetLdrDataTableEntry( list_head, name );

	if( !memory::IsAddressValid( ldr_data_table_entry ) )
	{
		TRACE( "%s: GetLdrDataTableEntry( ... ) error!", __FUNCTION__ );
		return 0;
	}

	return memory::ToAddress( ldr_data_table_entry->DllBase );
}

std::uintptr_t GetSystemImage( std::uint64_t name_hash )
{
	const auto list_head = GetPsLoadedModuleList();
	const auto ldr_data_table_entry = GetLdrDataTableEntry( list_head, name_hash );

	if( !memory::IsAddressValid( ldr_data_table_entry ) )
	{
		TRACE( "%s: GetLdrDataTableEntry( ... ) error!", __FUNCTION__ );
		return 0;
	}

	return memory::ToAddress( ldr_data_table_entry->DllBase );
}

std::uintptr_t GetKernelImage( const wchar_t* const name )
{
	return GetSystemImage( name );
}

std::uintptr_t GetKernelImage( std::uint64_t name_hash )
{
	return GetSystemImage( name_hash );
}

std::uintptr_t GetImageExport( std::uintptr_t image, const char* const name )
{
	const auto image_data_directory = GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_EXPORT );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_EXPORT ) error!", __FUNCTION__ );
		return 0;
	}

	if( !image_data_directory->VirtualAddress )
	{
		TRACE( "%s: image_data_directory->VirtualAddress is not valid!", __FUNCTION__ );
		return 0;
	}

	if( !image_data_directory->Size )
	{
		TRACE( "%s: image_data_directory->Size is not valid!", __FUNCTION__ );
		return 0;
	}

	const auto image_export_directory = reinterpret_cast< const IMAGE_EXPORT_DIRECTORY* >( image + image_data_directory->VirtualAddress );

	const auto address_of_functions = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfFunctions );
	const auto address_of_names = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfNames );
	const auto address_of_name_ordinals = reinterpret_cast< const std::uint16_t* >( image + image_export_directory->AddressOfNameOrdinals );

	for( std::uint32_t index = 0; index < image_export_directory->NumberOfFunctions; index++ )
	{
		const auto export_ordinal = address_of_name_ordinals[ index ];
		const auto export_procedure = address_of_functions[ export_ordinal ];

		if( !export_procedure )
		{
			continue;
		}

		const auto export_name = reinterpret_cast< const char* >( image + address_of_names[ index ] );

		if( !memory::IsAddressValid( export_name ) )
		{
			continue;
		}

		if( constant::AreEqual( export_name, name, false ) )
		{
			return ( image + export_procedure );
		}
	}

	return 0;
}

std::uintptr_t GetImageExport( std::uintptr_t image, std::uint64_t name_hash )
{
	const auto image_data_directory = GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_EXPORT );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_EXPORT ) error!", __FUNCTION__ );
		return 0;
	}

	if( !image_data_directory->VirtualAddress )
	{
		TRACE( "%s: image_data_directory->VirtualAddress is not valid!", __FUNCTION__ );
		return 0;
	}

	if( !image_data_directory->Size )
	{
		TRACE( "%s: image_data_directory->Size is not valid!", __FUNCTION__ );
		return 0;
	}

	const auto image_export_directory = reinterpret_cast< const IMAGE_EXPORT_DIRECTORY* >( image + image_data_directory->VirtualAddress );

	const auto address_of_functions = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfFunctions );
	const auto address_of_names = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfNames );
	const auto address_of_name_ordinals = reinterpret_cast< const std::uint16_t* >( image + image_export_directory->AddressOfNameOrdinals );

	for( std::uint32_t index = 0; index < image_export_directory->NumberOfFunctions; index++ )
	{
		const auto export_ordinal = address_of_name_ordinals[ index ];
		const auto export_procedure = address_of_functions[ export_ordinal ];

		if( !export_procedure )
		{
			continue;
		}

		const auto export_name = reinterpret_cast< const char* >( image + address_of_names[ index ] );

		if( !memory::IsAddressValid( export_name ) )
		{
			continue;
		}

		if( constant::Hash( export_name, true ) == name_hash )
		{
			return ( image + export_procedure );
		}
	}

	return 0;
}

} // namespace atom::win32