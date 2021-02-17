#include "../atom/system_map_data.hpp"

#include "../core/no_copy.hpp"
#include "../core/no_move.hpp"
#include "../core/singleton.hpp"
#include "../core/vector.hpp"

#include "../io/packet.hpp"
#include "../io/control.hpp"

#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../kernel/erase.hpp"
#include "../kernel/import.hpp"

#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../win32/sdk/client_id.hpp"
#include "../win32/sdk/image_data_directory.hpp"
#include "../win32/sdk/image_dos_header.hpp"
#include "../win32/sdk/image_export_directory.hpp"
#include "../win32/sdk/image_file_header.hpp"
#include "../win32/sdk/image_nt_headers.hpp"
#include "../win32/sdk/image_optional_header.hpp"
#include "../win32/sdk/image_section_header.hpp"
#include "../win32/sdk/ldr_data_table_entry.hpp"
#include "../win32/sdk/list_entry.hpp"
#include "../win32/sdk/nt_tib.hpp"
#include "../win32/sdk/peb.hpp"
#include "../win32/sdk/peb_ldr_data.hpp"
#include "../win32/sdk/string.hpp"
#include "../win32/sdk/teb.hpp"
#include "../win32/sdk/unicode_string.hpp"

#include "../win32/image.hpp"
#include "../win32/process.hpp"
#include "../win32/trace.hpp"

#include "dispatch_hook.hpp"
#include "image_wipe_header.hpp"

extern "C"
{
	// 
	// driver entry point ( invoked on load )
	// 
	NTSTATUS DriverEntry( void* driver_object, void* registry_path );
	// NTSTATUS DriverInitialize( DRIVER_OBJECT* driver_object, UNICODE_STRING* registry_path );

}; // extern "C"

// 
// use 'INIT' section for these
// 
#pragma alloc_text( INIT, DriverEntry )

void* g_hook = nullptr;
std::uint8_t* g_hook_restore = nullptr;

namespace atom::win32
{

enum FUNCTION_TABLE_TYPE
{
	RF_SORTED = 0,
	RF_UNSORTED = 1,
	RF_CALLBACK = 2,
	RF_KERNEL_DYNAMIC = 3,
};

struct RTL_BALANCED_NODE
{
	union
	{
		RTL_BALANCED_NODE* Children[ 2 ] = { };
		struct
		{
			RTL_BALANCED_NODE* Left;
			RTL_BALANCED_NODE* Right;
		};
	};
	union
	{
		std::uint8_t Red : 1;
		std::uint8_t Balance : 2;
		std::uint64_t ParentValue = 0;
	};
};

struct IMAGE_RUNTIME_FUNCTION_ENTRY
{
	std::uint32_t BeginAddress = 0;
	std::uint32_t EndAddress = 0;
	union
	{
		std::uint32_t UnwindInfoAddress = 0;
		std::uint32_t UnwindData;
	};
};

struct DYNAMIC_FUNCTION_TABLE
{
	LIST_ENTRY ListEntry = { };
	IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable = nullptr;
	LARGE_INTEGER TimeStamp = { };
	std::uint64_t MinimumAddress = 0;
	std::uint64_t MaximumAddress = 0;
	std::uint64_t BaseAddress = 0;
	void* Callback = nullptr;
	void* Context = nullptr;
	wchar_t* OutOfProcessCallbackDll = nullptr;
	FUNCTION_TABLE_TYPE Type = RF_SORTED;
	std::uint32_t EntryCount = 0;
	RTL_BALANCED_NODE TreeNode = { };
};

struct INVERTED_FUNCTION_TABLE_ENTRY
{
	union
	{
		IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable = nullptr;
		DYNAMIC_FUNCTION_TABLE* DynamicTable;
	};
	void* ImageBase = nullptr;
	std::uint32_t SizeOfImage = 0;
	std::uint32_t SizeOfTable = 0;
};

struct INVERTED_FUNCTION_TABLE
{
	std::uint32_t CurrentSize = 0;
	std::uint32_t MaximumSize = 0;
	std::uint32_t Epoch = 0;
	std::uint8_t Overflow = 0;
	std::uint8_t Reserved[ 3 ] = { };
	INVERTED_FUNCTION_TABLE_ENTRY TableEntry[ 256 ] = { };
};

} // namespace atom::win32

namespace atom
{

void TestExceptionSupport()
{
	__try
	{
		const auto image_base = g_map_data.m_base;
		TRACE( "%s: image_base = '0x%016llX'", __FUNCTION__, image_base );

		const auto dereferenced = *reinterpret_cast< std::uint64_t* >( image_base );
		TRACE( "%s: dereferenced = '0x%016llX'", __FUNCTION__, dereferenced );

		const auto dereferenced_second = *reinterpret_cast< std::uint64_t* >( dereferenced );
		TRACE( "%s: dereferenced_second = '0x%016llX'", __FUNCTION__, dereferenced_second );

		const auto image_dos_header = reinterpret_cast< const win32::IMAGE_DOS_HEADER* >( image_base );
		const auto image_nt_headers = reinterpret_cast< const win32::IMAGE_NT_HEADERS* >( image_base + image_dos_header->e_lfanew );

		const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

		for( std::uint16_t index = 0; index < image_nt_headers->FileHeader.NumberOfSections; index++ )
		{
			const auto image_section = &image_section_header[ index ];

			char image_section_name[ 16 ] = { };
			std::memcpy( image_section_name, image_section->Name, sizeof( image_section->Name ) );

			TRACE( "%s = '0x%08X, 0x%08X'", image_section_name, image_section->VirtualAddress, image_section->Misc.VirtualSize );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		// DbgPrint( "[atom::doze] exception occurred!\n" );
		TRACE( "%s: Exception occurred! (0x%08X)", __FUNCTION__, GetExceptionCode() );
	}
}

void TestInvertedFunctionTable()
{
	

	__try
	{
		auto PsInvertedFunctionTable = reinterpret_cast< win32::INVERTED_FUNCTION_TABLE* >( g_map_data.m_symbols.PsInvertedFunctionTable );

		if( PsInvertedFunctionTable )
		{
			TRACE( "PsInvertedFunctionTable->CurrentSize = '0x%08X'", PsInvertedFunctionTable->CurrentSize );
			TRACE( "PsInvertedFunctionTable->MaximumSize = '0x%08X'", PsInvertedFunctionTable->MaximumSize );
			TRACE( "PsInvertedFunctionTable->Epoch = '0x%08X'", PsInvertedFunctionTable->Epoch );
			TRACE( "PsInvertedFunctionTable->Overflow = '%s'", PsInvertedFunctionTable->Overflow ? "true" : "false" );
			TRACE_SEPARATOR();

			for( std::uint32_t Index = 0; Index < PsInvertedFunctionTable->CurrentSize; Index++ )
			{
				auto TableEntry = &PsInvertedFunctionTable->TableEntry[ Index ];

				if( TableEntry->ImageBase == memory::ToPointer( g_map_data.m_base ) )
				{
					TRACE( "TableEntry[%u]->FunctionTable = '0x%016llX'", Index, memory::ToAddress( TableEntry->FunctionTable ) );
					TRACE( "TableEntry[%u]->ImageBase = '0x%016llX'", Index, memory::ToAddress( TableEntry->ImageBase ) );
					TRACE( "TableEntry[%u]->SizeOfImage = '0x%08X'", Index, TableEntry->SizeOfImage );
					TRACE( "TableEntry[%u]->SizeOfTable = '0x%08X'", Index, TableEntry->SizeOfTable );
					TRACE_SEPARATOR();
				}
			}
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		TRACE( "%s: Exception occurred! (0x%08X)", __FUNCTION__, GetExceptionCode() );
	}
}

NTSTATUS DriverDispatch( void* driver_object )
{
	auto status = STATUS_SUCCESS;

	__try
	{
		const auto driver_map_context = static_cast< SystemMapData* >( driver_object );

		if( !memory::IsAddressValid( driver_map_context ) )
		{
			TRACE( "%s: driver_map_context is not a valid address!", __FUNCTION__ );
			return status = STATUS_INVALID_PARAMETER;
		}

		std::memcpy( &g_map_data, driver_map_context, sizeof( g_map_data ) );

		if( !g_map_data.IsValid() )
		{
			TRACE( "%s: g_map_data is not valid!", __FUNCTION__ );
			return status = STATUS_INVALID_PARAMETER;
		}

		if( !ImageWipeHeader( g_map_data.m_base ) )
		{
			TRACE( "%s: ImageWipeHeader( '0x%016llX' ) error!", __FUNCTION__ );
		}

		status = HookDispatch( DEVICE_LANMAN_DATAGRAM_RECEIVER );

		if( !NT_SUCCESS( status ) )
		{
			TRACE( "%s: HookDispatch( '%S' ) error! (0x%08X)", __FUNCTION__, DEVICE_LANMAN_DATAGRAM_RECEIVER, status );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		status = GetExceptionCode();
		TRACE( "%s: Exception occurred! (0x%08X)", __FUNCTION__, status );
	}

	return status;
}

} // namespace atom

// 
// driver entry point
// 
NTSTATUS DriverEntry( void* driver_object, void* registry_path )
{
	UNREFERENCED_PARAMETER( registry_path );
	return atom::DriverDispatch( driver_object );
}