#include "mmap.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../core/map_data.hpp"

#include "../core/string.hpp"

#include "../io/driver_control.hpp"

#include "../memory/operation.hpp"

#include "../options/configuration.hpp"

#include "../win32/console.hpp"
#include "../win32/file.hpp"
#include "../win32/handle_guard.hpp"

// #include "../asmjit/x86.h"

namespace atom
{

/*MMap::MMap()
	: m_runtime()
	, m_code()
	, m_assembler( &m_code )
{
	m_code.init( m_runtime.environment() );
}

MMap::~MMap()
{
	Destroy();
}

bool MMap::Create( const std::wstring& class_name )
{
	Destroy();

	m_class_name = class_name;

	if( m_class_name.empty() )
	{
		TRACE( "%s: m_class_name is empty!", ATOM_FUNCTION );
		return false;
	}

	m_window = FindWindowW( m_class_name.c_str(), nullptr );

	if( !m_window )
	{
		TRACE( "%s: FindWindowW( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
		return false;
	}

	m_thread_id = GetWindowThreadProcessId( m_window, reinterpret_cast< DWORD* >( &m_process_id ) );

	if( !m_thread_id || !m_process_id )
	{
		TRACE( "%s: GetWindowThreadProcessId( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
		return false;
	}

	return true;
}

void MMap::Destroy()
{
	m_class_name.clear();
	m_window = nullptr;
	m_thread_id = 0;
	m_process_id = 0;
}

bool MMap::MapImage( const std::wstring& name )
{
	wchar_t current_directory[ MAX_PATH ] = { };
	
	if( !GetCurrentDirectoryW( MAX_PATH, current_directory ) )
	{
		TRACE( "%s: GetCurrentDirectory( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
		return false;
	}

	std::wstring directory( current_directory );
	
	directory.append( L"\\" );
	directory.append( name );

	std::vector< std::uint8_t > image = { };

	if( !win32::SafeReadFile( directory, image ) )
	{
		TRACE( "%s: win32::SafeReadFile( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
		return false;
	}

	return MapImage( image.data(), image.size() );
}

void PrepareMapData( core::MapData* map_data );

bool MMap::MapImage( const std::uint8_t* const data, std::size_t size )
{
	const auto image_dos_header = reinterpret_cast< const IMAGE_DOS_HEADER* >( data );

	if( image_dos_header->e_magic != IMAGE_DOS_SIGNATURE )
	{
		TRACE( "%s: Invalid image 'DOS' signature!", ATOM_FUNCTION );
		return false;
	}

	const auto image_nt_headers = reinterpret_cast< const IMAGE_NT_HEADERS* >( data + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != IMAGE_NT_SIGNATURE )
	{
		TRACE( "%s: Invalid image 'NT' signature!", ATOM_FUNCTION );
		return false;
	}

	m_image_data = std::make_unique< std::uint8_t[] >( image_nt_headers->OptionalHeader.SizeOfImage );
	m_image_base = reinterpret_cast< std::uintptr_t >( m_image_data.get() );
	m_image_size = static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage );

	m_image_dos_header = reinterpret_cast< IMAGE_DOS_HEADER* >( m_image_base );
	m_image_nt_headers = reinterpret_cast< IMAGE_NT_HEADERS* >( m_image_base + m_image_dos_header->e_lfanew );

	if( !CopyHeaders( data ) )
	{
		TRACE( "%s: CopyHeaders( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	if( !CopySections( data ) )
	{
		TRACE( "%s: CopySections( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	if( !ProcessRelocations( data ) )
	{
		TRACE( "%s: ProcessRelocations( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	auto& doze = io::DriverControl::Instance();

	auto user_image = doze.Commit( m_image_size, PAGE_READWRITE );
	
	if( !user_image.IsValid() )
	{
		TRACE( "%s: doze.Commit( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	if( !user_image.Write( m_image_data.get(), m_image_size ) )
	{
		TRACE( "%s: user_image.Write( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	core::MapData map_data = { };
	map_data.m_base = user_image.m_address;
	map_data.m_size = user_image.m_size;
	PrepareMapData( &map_data );

	auto user_map_data = doze.Commit( sizeof( map_data ), PAGE_READWRITE );

	if( !user_map_data.IsValid() )
	{
		TRACE( "%s: doze.Commit( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	if( !user_map_data.Write( &map_data ) )
	{
		TRACE( "%s: user_map_data.Write( ... ) error!", ATOM_FUNCTION );
		return false;
	}

	auto user_image_entry = ( user_image.m_address + m_image_nt_headers->OptionalHeader.AddressOfEntryPoint );

	std::vector< std::uint64_t > arg_pack = { };
	arg_pack.emplace_back( user_image.m_address );
	arg_pack.emplace_back( DLL_PROCESS_ATTACH );
	arg_pack.emplace_back( user_map_data.m_address );

	GenerateBegin();

	GenerateCall( user_image_entry, arg_pack );
	GenerateEnd();
}

bool MMap::CopyHeaders( const std::uint8_t* const data )
{
	// 
	// copy image pe headers
	// 
	std::memcpy( m_image_data.get(), data, m_image_nt_headers->OptionalHeader.SizeOfHeaders );
	return true;
}

bool MMap::CopySections( const std::uint8_t* const data )
{
	if( !m_image_nt_headers->FileHeader.NumberOfSections )
	{
		TRACE( "%s: Image has no sections!", ATOM_FUNCTION );
		return false;
	}

	const auto image_section_header = IMAGE_FIRST_SECTION( m_image_nt_headers );

	for( std::uint16_t index = 0; index < m_image_nt_headers->FileHeader.NumberOfSections; index++ )
	{
		const auto image_section = &image_section_header[ index ];

		if( image_section->VirtualAddress &&
				image_section->PointerToRawData &&
				image_section->SizeOfRawData )
		{
			char name[ 16 ] = { };
			std::memcpy( name, image_section->Name, sizeof( image_section->Name ) );

			if( constant::AreEqual( name, ".reloc", true ) )
			{
				// 
				// skip relocations section
				// 
				continue;
			}

			const auto destination = m_image_data.get() + image_section->VirtualAddress;
			const auto source = data + image_section->PointerToRawData;
			const auto size = static_cast< std::size_t >( image_section->SizeOfRawData );

			// 
			// copy image section data
			// 
			std::memcpy( destination, source, size );
		}
	}

	return true;
}

bool MMap::ProcessRelocations( const std::uint8_t* const data )
{
	auto disp = ( m_image_base - m_image_nt_headers->OptionalHeader.ImageBase );

	if( disp == 0 )
	{
		TRACE( "%s: No need for relocation!", ATOM_FUNCTION );
		return true;
	}

	if( !( m_image_nt_headers->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE ) )
	{
		TRACE( "%s: Can't relocate image, no relocation flag!", ATOM_FUNCTION );
		return false;
	}

	auto image_data_directory = &m_image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ];

	auto start = m_image_base + image_data_directory->VirtualAddress;
	auto end = start + image_data_directory->Size;

	struct ImageBaseRelocation
	{
		std::uint32_t VirtualAddress = 0;
		std::uint32_t SizeOfBlock = 0;
		struct
		{
			std::uint16_t Offset : 12;
			std::uint16_t Type : 4;
		} TypeOffset[ 1 ] = { };
	};

	auto relocation = reinterpret_cast< ImageBaseRelocation* >( start );

	while( std::uintptr_t( relocation ) < end && relocation->SizeOfBlock )
	{
		auto count = ( ( relocation->SizeOfBlock - 8 ) >> 1 );

		for( std::uint32_t index = 0; index < count; index++ )
		{
			auto type = ( relocation->TypeOffset[ index ].Type );
			auto offset = ( relocation->TypeOffset[ index ].Offset ) % 4096;

			if( type == IMAGE_REL_BASED_ABSOLUTE )
			{
				continue;
			}

			if( type == IMAGE_REL_BASED_DIR64 || 
					type == IMAGE_REL_BASED_HIGHLOW )
			{
				auto rel = offset + relocation->VirtualAddress;
				auto value = *reinterpret_cast< std::uint64_t* >( m_image_base + rel ) + disp;
				*reinterpret_cast< std::uint64_t* >( m_image_base + rel ) = value;
			}
		}

		relocation = reinterpret_cast< ImageBaseRelocation* >( std::uintptr_t( relocation ) + relocation->SizeOfBlock );
	}

	return true;
}

void MMap::GenerateBegin()
{
	m_assembler.mov( qword_ptr( x86::rsp, 1 * sizeof( std::uint64_t ) ), x86::rcx );
	m_assembler.mov( qword_ptr( x86::rsp, 2 * sizeof( std::uint64_t ) ), x86::rdx );
	m_assembler.mov( qword_ptr( x86::rsp, 3 * sizeof( std::uint64_t ) ), x86::r8 );
	m_assembler.mov( qword_ptr( x86::rsp, 4 * sizeof( std::uint64_t ) ), x86::r9 );
}

void MMap::GenerateArg( std::size_t index, std::uint64_t arg )
{
	if( index < 4 )
	{
		m_assembler.mov( cx_reg_pack[ index ], arg );
	}
	else
	{
		m_assembler.mov( x86::rax, arg );
		m_assembler.mov( qword_ptr( x86::rsp, index * sizeof( std::uint64_t ) ), x86::rax );
	}
}

void MMap::GenerateCall( std::uintptr_t procedure, const std::vector< std::uint64_t >& arg_pack )
{
	const auto arg_count = arg_pack.size();
	const auto arg_stack = ( ( arg_count > 4 ) ? ( arg_count * sizeof( std::uint64_t ) ) : 40 );

	// 
	// reserve stack space
	// 
	m_assembler.sub( x86::rsp, arg_stack + 8 );

	// 
	// pass every arg
	// 
	for( std::size_t index = 0; index < arg_count; index++ )
	{
		const auto arg = arg_pack[ index ];
		GenerateArg( index, arg );
	}

	// 
	// store procedure in rax register,
	// and try to call it
	// 
	m_assembler.mov( x86::rax, procedure );
	m_assembler.call( x86::rax );

	// 
	// restore stack space
	// 
	m_assembler.add( x86::rsp, arg_stack + 8 );
}

void MMap::GenerateEnd()
{
	m_assembler.mov( x86::rcx, qword_ptr( x86::rsp, 1 * sizeof( std::uint64_t ) ) );
	m_assembler.mov( x86::rdx, qword_ptr( x86::rsp, 2 * sizeof( std::uint64_t ) ) );
	m_assembler.mov( x86::r8, qword_ptr( x86::rsp, 3 * sizeof( std::uint64_t ) ) );
	m_assembler.mov( x86::r9, qword_ptr( x86::rsp, 4 * sizeof( std::uint64_t ) ) );
	m_assembler.ret();
}*/

} // namespace atom

namespace atom
{

struct ProcessRecord
{
	bool Valid() const
	{
		if( !m_process_id )
			return false;

		if( !m_thread_count )
			return false;

		return true;
	}

	std::uint32_t m_process_id = 0;
	std::uint32_t m_thread_count = 0;
};

bool GetProcessIdByName( const std::wstring& name, std::vector< ProcessRecord >& process_array )
{
	process_array.clear();

	if( name.empty() )
	{
		HORIZON_LOG( "%s: name is empty!", __FUNCTION__ );
		return false;
	}

	win32::Handle snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if( !snapshot.IsValid() )
	{
		HORIZON_LOG( "%s: CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	PROCESSENTRY32W entry =
	{
		sizeof( PROCESSENTRY32W ),
	};

	if( !Process32FirstW( snapshot, &entry ) )
	{
		HORIZON_LOG( "%s: Process32FirstW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	do
	{
		std::wstring process_name( entry.szExeFile );

		if( process_name.empty() )
			continue;

		if( core::ToLower( process_name ) == core::ToLower( name ) )
		{
			const auto record = ProcessRecord
			{
				static_cast< std::uint32_t >( entry.th32ProcessID ),
				static_cast< std::uint32_t >( entry.cntThreads ),
			};

			process_array.emplace_back( record );
		}
	}
	while( Process32NextW( snapshot, &entry ) );

	return !process_array.empty();
}

std::uint32_t GetProcessIdWithMaxThreadCount( const std::wstring& name )
{
	ProcessRecord process_record = { };

	while( true )
	{
		std::vector< ProcessRecord > processes = { };

		if( GetProcessIdByName( name, processes ) )
		{
			for( const auto& process : processes )
			{
				if( process.m_thread_count > process_record.m_thread_count )
				{
					process_record = process;
				}
			}

			break;
		}

		std::this_thread::sleep_for( 50ms );
	}

	return process_record.m_process_id;
}

IMAGE_NT_HEADERS* GetImageNtHeaders( std::uintptr_t image )
{
	if( !image )
		return nullptr;

	const auto image_dos_header = reinterpret_cast< IMAGE_DOS_HEADER* >( image );

	if( image_dos_header->e_magic != IMAGE_DOS_SIGNATURE )
		return nullptr;

	const auto image_nt_headers = reinterpret_cast< IMAGE_NT_HEADERS* >( image + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != IMAGE_NT_SIGNATURE )
		return nullptr;

	return image_nt_headers;
}

bool MapImageHeader( std::uintptr_t image, std::uintptr_t image_data )
{
	const auto image_nt_headers = GetImageNtHeaders( image_data );

	if( !image_nt_headers )
	{
		return false;
	}

	const auto image_size = image_nt_headers->OptionalHeader.SizeOfHeaders;

	HORIZON_LOG( "%s: [0x%016llX] -> [0x%016llX] (0x%08X)", __FUNCTION__, image_data, image, image_size );

	const auto image_pointer = memory::ToPointer( image );
	const auto image_data_pointer = memory::ToPointer( image_data );

	std::memcpy( image_pointer, image_data_pointer, image_size );

	return true;
}

bool MapImageSection( std::uintptr_t image, std::uintptr_t image_data )
{
	const auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
		return false;

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	if( !image_section_header )
		return false;

	for( std::uint16_t i = 0; i < image_nt_headers->FileHeader.NumberOfSections; i++ )
	{
		const auto image_section = &image_section_header[ i ];

		if( !image_section )
			continue;

		char section_name[ 9 ] = { };
		std::memcpy( section_name, image_section->Name, sizeof( image_section->Name ) );

		HORIZON_LOG( "%s: \"%s\" [0x%016llX] -> [0x%016llX] (0x%08X)", __FUNCTION__, section_name, image_data + image_section->PointerToRawData, image + image_section->VirtualAddress, image_section->SizeOfRawData );

		const auto destination = memory::ToPointer( image + image_section->VirtualAddress );
		const auto source = memory::ToPointer( image_data + image_section->PointerToRawData );
		const auto size = static_cast< std::size_t >( image_section->SizeOfRawData );

		std::memcpy( destination, source, size );
	}

	return true;
}

struct ImageRelocationData
{
	std::uintptr_t m_address = 0;
	std::uint16_t* m_item = nullptr;
	std::uint32_t m_count = 0;
};

struct ImageRelocationEntry
{
	std::uint32_t VirtualAddress;
	std::uint32_t SizeOfBlock;
	std::uint16_t TypeOffset[ 1 ] = { };
};

bool MapImageRelocation( std::uintptr_t image, std::uintptr_t image_data, IMAGE_NT_HEADERS* image_nt_headers )
{
	const auto image_data_directory = image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ];

	if( !image_data_directory.VirtualAddress || !image_data_directory.Size )
	{
		HORIZON_LOG( "%s: No relocations...", __FUNCTION__ );
		return true;
	}

	auto base_relocation = reinterpret_cast< IMAGE_BASE_RELOCATION* >( image_data + image_data_directory.VirtualAddress );
	auto base_relocation_end = memory::ToAddress( base_relocation ) + image_data_directory.Size;

	while( base_relocation && base_relocation->VirtualAddress && base_relocation->SizeOfBlock && base_relocation->VirtualAddress < base_relocation_end )
	{
		ImageRelocationData image_relocation =
		{
			image_data + base_relocation->VirtualAddress,
			reinterpret_cast< std::uint16_t* >( memory::ToAddress( base_relocation ) + sizeof( IMAGE_BASE_RELOCATION ) ),
			static_cast< std::uint32_t >( ( base_relocation->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( std::uint16_t ) ),
		};

		for( auto i = 0u; i < image_relocation.m_count; i++ )
		{
			const auto type = static_cast< std::uint16_t >( image_relocation.m_item[ i ] >> 12 );
			const auto offset = static_cast< std::uint16_t >( image_relocation.m_item[ i ] & 0xFFF );

			if( type == IMAGE_REL_BASED_HIGHLOW ||
					type == IMAGE_REL_BASED_DIR64 )
			{
				HORIZON_LOG( "%s: [0x%016llX] [0x%04X] [0x%04X] (0x%016llX)", __FUNCTION__, image_relocation.m_address, type, offset, image - image_nt_headers->OptionalHeader.ImageBase );
				*reinterpret_cast< std::uintptr_t* >( image_relocation.m_address + offset ) += ( image - image_nt_headers->OptionalHeader.ImageBase );
			}
		}

		base_relocation = reinterpret_cast< IMAGE_BASE_RELOCATION* >( memory::ToAddress( base_relocation ) + base_relocation->SizeOfBlock );
	}

	return true;
}

std::uintptr_t GetRemoteImageExport( std::uintptr_t image, const std::string& name )
{
	auto& driver = io::DriverControl::Instance();

	if( !image )
	{
		HORIZON_LOG( "%s: image is 0!", __FUNCTION__ );
		return 0;
	}

	const auto image_dos_header = driver.Read< IMAGE_DOS_HEADER >( image );

	if( image_dos_header.e_magic != IMAGE_DOS_SIGNATURE )
	{
		HORIZON_LOG( "%s: Not a valid DOS image!", __FUNCTION__ );
		return 0;
	}

	const auto image_nt_headers = driver.Read< IMAGE_NT_HEADERS >( image + image_dos_header.e_lfanew );

	if( image_nt_headers.Signature != IMAGE_NT_SIGNATURE )
	{
		HORIZON_LOG( "%s: Not a valid NT image!", __FUNCTION__ );
		return 0;
	}

	const auto& directory_entry_export = image_nt_headers.OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];

	if( !directory_entry_export.VirtualAddress || !directory_entry_export.Size )
	{
		HORIZON_LOG( "%s: Not a valid image export directory!", __FUNCTION__ );
		return 0;
	}

	const auto image_export_directory_data = std::make_unique< std::uint8_t[] >( directory_entry_export.Size );

	if( !driver.ReadMemory( image + directory_entry_export.VirtualAddress, image_export_directory_data.get(), directory_entry_export.Size ) )
	{
		HORIZON_LOG( "%s: ReadMemory( 0x%016llX, 0x%08X ) error!", __FUNCTION__, image + directory_entry_export.VirtualAddress, directory_entry_export.Size );
		return 0;
	}

	const auto image_export_directory = PIMAGE_EXPORT_DIRECTORY( image_export_directory_data.get() );
	const auto image_export_displacement = std::uintptr_t( image_export_directory_data.get() ) - directory_entry_export.VirtualAddress;

	const auto address_of_functions = reinterpret_cast< std::uint32_t* >( image_export_displacement + image_export_directory->AddressOfFunctions );
	const auto address_of_names = reinterpret_cast< std::uint32_t* >( image_export_displacement + image_export_directory->AddressOfNames );
	const auto address_of_name_ordinals = reinterpret_cast< std::uint16_t* >( image_export_displacement + image_export_directory->AddressOfNameOrdinals );

	for( auto i = 0ul; i < image_export_directory->NumberOfNames; i++ )
	{
		const auto export_name = std::string( PCHAR( image_export_displacement + address_of_names[ i ] ) );

		if( core::ToLower( export_name ) == core::ToLower( name ) )
		{
			const auto export_ordinal = address_of_name_ordinals[ i ];
			return ( image + address_of_functions[ export_ordinal ] );
		}
	}

	HORIZON_LOG( "%s: Image export \"%s\" is not found in [0x%016llX]!", __FUNCTION__, name.c_str(), image );
	return 0;
}

bool MapImageImportDescriptor( std::uintptr_t image )
{
	auto& driver = io::DriverControl::Instance();

	const auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
		return false;

	const auto image_data_directory = image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ];

	if( !image_data_directory.VirtualAddress || !image_data_directory.Size )
		return true;

	auto import_descriptor = reinterpret_cast< IMAGE_IMPORT_DESCRIPTOR* >( image + image_data_directory.VirtualAddress );

	while( import_descriptor && import_descriptor->Name )
	{
		const auto import_descriptor_name = reinterpret_cast< const char* >( image + import_descriptor->Name );
		const auto import_descriptor_name_wide = core::ToWideString( import_descriptor_name );
		const auto import_descriptor_image = driver.GetImage( import_descriptor_name_wide.c_str() );

		if( import_descriptor_image )
		{
			auto thunk_data = reinterpret_cast< IMAGE_THUNK_DATA* >( image + import_descriptor->FirstThunk );

			while( thunk_data && thunk_data->u1.AddressOfData )
			{
				if( !( thunk_data->u1.Ordinal & IMAGE_ORDINAL_FLAG64 ) )
				{
					const auto import_by_name = reinterpret_cast< IMAGE_IMPORT_BY_NAME* >( image + thunk_data->u1.AddressOfData );
					const auto import_procedure = GetRemoteImageExport( import_descriptor_image, import_by_name->Name );

					if( import_procedure )
					{
						TRACE( "%s: %s: [0x%016llX]", __FUNCTION__, import_by_name->Name, import_procedure );
						thunk_data->u1.Function = import_procedure;
					}
				}

				thunk_data++;
			}
		}

		import_descriptor++;
	}

	return true;
}

struct ImageMapData
{
	std::uintptr_t m_base = 0;
	std::uint32_t m_size = 0;

	std::uintptr_t m_swap_chain = 0;
	std::uintptr_t m_swap_chain_restore = 0;

	std::uint32_t m_nt_query_virtual_memory = 0;
};

#define LOG		HORIZON_LOG

#pragma pack( push, 8 )

struct ExecuteData
{
	std::uint32_t m_status = 0;			// 0x0000
	
	std::uint64_t m_procedure = 0;	// 0x0008
	
	std::uint64_t m_rcx = 0;				// 0x0010
	std::uint64_t m_rdx = 0;				// 0x0018
	std::uint64_t m_r8 = 0;					// 0x0020

	std::uint64_t m_rax = 0;				// 0x0028
};
// sizeof( ExecuteData ) = 0x0030

#pragma pack( pop )

std::uint8_t g_execute_code[] =
{
	/* 0x0000 */ 0x51,																												// push rcx
	/* 0x0001 */ 0x52,																												// push rdx
	/* 0x0002 */ 0x53,																												// push rbx
	/* 0x0003 */ 0x55,																												// push rbp
	/* 0x0004 */ 0x56,																												// push rsi
	/* 0x0005 */ 0x57,																												// push rdi
	/* 0x0006 */ 0x41, 0x50,																									// push r8
	/* 0x0008 */ 0x41, 0x51,																									// push r9
	/* 0x000A */ 0x41, 0x52,																									// push r10
	/* 0x000C */ 0x41, 0x53,																									// push r11
	/* 0x000E */ 0x41, 0x54,																									// push r12
	/* 0x0010 */ 0x41, 0x55,																									// push r13
	/* 0x0012 */ 0x41, 0x56,																									// push r14
	/* 0x0014 */ 0x41, 0x57,																									// push r15
	/* 0x0016 */ 0x48, 0x83, 0xEC, 0x38,																			// sub rsp, 38h
	/* 0x001A */ 0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov r10, 0000000000000000h
	/* 0x0024 */ 0x4C, 0x89, 0x54, 0x24, 0x20,																// mov [ rsp + 20h ], r10
	/* 0x0029 */ 0x41, 0x83, 0x3A, 0x00,																			// cmp dword ptr [ r10 ], 00h
	/* 0x002D */ 0x75, 0x30,																									// jne 30h
	/* 0x002F */ 0x41, 0xC7, 0x02, 0x01, 0x00, 0x00, 0x00,										// mov [ r10 ], 00000001h
	/* 0x0036 */ 0x49, 0x8B, 0x42, 0x08,																			// mov rax, [ r10 + 08h ]
	/* 0x003A */ 0x48, 0x89, 0x44, 0x24, 0x28,																// mov [ rsp + 28h ], rax
	/* 0x003F */ 0x4D, 0x8B, 0x42, 0x20,																			// mov r8, [ r10 + 20h ]
	/* 0x0043 */ 0x49, 0x8B, 0x52, 0x18,																			// mov rdx, [ r10 + 18h ]
	/* 0x0047 */ 0x49, 0x8B, 0x4A, 0x10,																			// mov rcx, [ r10 + 10h ]
	/* 0x004B */ 0xFF, 0x54, 0x24, 0x28,																			// call qword ptr [ rsp + 28h ]
	/* 0x004F */ 0x4C, 0x8B, 0x54, 0x24, 0x20,																// mov r10, [ rsp + 20h ]
	/* 0x0054 */ 0x49, 0x89, 0x42, 0x28,																			// mov [ r10 + 28h ], rax
	/* 0x0058 */ 0x41, 0xC7, 0x02, 0x02, 0x00, 0x00, 0x00,										// mov [ r10 ], 00000002h
	/* 0x005F */ 0x48, 0x83, 0xC4, 0x38,																			// add rsp, 38h
	/* 0x0063 */ 0x41, 0x5F,																									// pop r15
	/* 0x0065 */ 0x41, 0x5E,																									// pop r14
	/* 0x0067 */ 0x41, 0x5D,																									// pop r13
	/* 0x0069 */ 0x41, 0x5C,																									// pop r12
	/* 0x006B */ 0x41, 0x5B,																									// pop r11
	/* 0x006D */ 0x41, 0x5A,																									// pop r10
	/* 0x006F */ 0x41, 0x59,																									// pop r9
	/* 0x0071 */ 0x41, 0x58,																									// pop r8
	/* 0x0073 */ 0x5F,																												// pop rdi
	/* 0x0074 */ 0x5E,																												// pop rsi
	/* 0x0075 */ 0x5D,																												// pop rbp
	/* 0x0076 */ 0x5B,																												// pop rbx
	/* 0x0077 */ 0x5A,																												// pop rdx
	/* 0x0078 */ 0x59,																												// pop rcx
	/* 0x0079 */ 0xC3,																												// ret
	/* 0x007A */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,													// int 3
};

struct CDM
{
	int m_status;							// 0x0000
	DWORD_PTR m_image_entry;	// 0x0008
	HINSTANCE m_image_base;		// 0x0010
};

std::uint8_t g_execute_image_entry[] =
{
	0x48, 0x83, 0xEC, 0x38,																			// sub rsp, 38h
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov rax, 0000000000000000h		; address of ExecuteProcedureData instance
	0x48, 0x89, 0x44, 0x24, 0x20,																// mov [ rsp + 20h ], rax				; dereference stack + 20h as qword and set it to rax
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]				; set rax to address of ExecuteProcedureData instance
	0x83, 0x38, 0x00,																						// cmp dword ptr [ rax ], 00h		; dereference rax as dword and compare to 0 ( m_status )
	0x75, 0x39,																									// jne 39h											; jump to +39h if not equals
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]				; 
	0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,													// mov [ rax ], 00000001h				; set m_status to 1
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]				; 
	0x48, 0x8B, 0x40, 0x08,																			// mov rax, [ rax + 08h ]				; set rax to m_procedure
	0x48, 0x89, 0x44, 0x24, 0x28,																// mov [ rsp + 28h ], rax				; dereference stack + 28h as qword and set it to rax
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]				; 
	0x44, 0x8B, 0x40, 0x20,																			// mov r8d, [ rax + 20h ]				; set r8d to m_r8
	0x8B, 0x50, 0x18,																						// mov edx, [ rax + 18h ]				; set edx to m_rdx
	0x48, 0x8B, 0x48, 0x10,																			// mov rcx, [ rax + 10h ]				; set rcx to m_rcx
	0xFF, 0x54, 0x24, 0x28,																			// call qword ptr [ rsp + 28h ]	;


	0x45, 0x33, 0xC0,																						// xor r8d, r8d
	0xBA, 0x01, 0x00, 0x00, 0x00,																// mov edx, 00000001h
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]
	0x48, 0x8B, 0x48, 0x10,																			// mov rcx, [ rax + 10h ]
	0xFF, 0x54, 0x24, 0x28,																			// call qword ptr [ rsp + 28h ]
	0x48, 0x8B, 0x44, 0x24, 0x20,																// mov rax, [ rsp + 20h ]
	0xC7, 0x00, 0x02, 0x00, 0x00, 0x00,													// mov [ rax ], 00000002h
	0x48, 0x83, 0xC4, 0x38,																			// add rsp, 38h
	0xC3,																												// ret
	0xCC,																												// int 3
	0xCC,																												// int 3
	0xCC,																												// int 3
	0xCC,																												// int 3
	0xCC,																												// int 3
};

#define STATUS_ENTRY_WAITING		0
#define STATUS_ENTRY_RETURNED		1
#define STATUS_TARGET_CLOSED		2

constexpr std::uint32_t StatusExecuteWait = 0;
constexpr std::uint32_t StatusExecuteEnter = 1;
constexpr std::uint32_t StatusExecuteLeave = 2;

std::uintptr_t GetImage( const wchar_t* const name )
{
	return memory::ToAddress( GetModuleHandleW( name ) );
}

std::uintptr_t GetImageExport( std::uintptr_t image, const char* const name )
{
	return std::uintptr_t( GetProcAddress( HMODULE( image ), name ) );
}

std::uintptr_t GetExport( std::uintptr_t image, const char* const name )
{
	const auto image_export = GetImageExport( image, name );

	if( image_export )
	{
		TRACE( "%s = '0x%016llX'", name, ( image_export - image ) );
		return ( image_export - image );
	}

	return 0;
}

constexpr std::uint32_t BadSystemNumber = std::numeric_limits< std::uint32_t >::max();

std::uint32_t GetSystemNumber( std::uintptr_t image, const char* const name )
{
	const auto procedure = GetImageExport( image, name );

	if( procedure )
	{
		const auto number = *reinterpret_cast< std::uint32_t* >( procedure + 0x04 );
		TRACE( "%s = '0x%08X'", name, number );
		return number;
	}

	return BadSystemNumber;
}

std::uint32_t GetSystemNumber( std::uintptr_t procedure )
{
	const auto head = *reinterpret_cast< std::uint32_t* >( procedure );

	if( head == 0xB8D18B4C )
	{
		const auto body = *reinterpret_cast< std::uint16_t* >( procedure + 0x0012 );

		if( body == 0x050F )
		{
			const auto number = *reinterpret_cast< std::uint32_t* >( procedure + 0x0004 );
			return number;
		}
	}

	return BadSystemNumber;
}

struct ExportData
{
	std::uint64_t m_name_hash = 0;
	std::uintptr_t m_address = 0;
};

struct SystemNumberData
{
	std::uint64_t m_name_hash = 0;
	std::uint32_t m_number = BadSystemNumber;
};

std::vector< ExportData > GetExports( std::uintptr_t image )
{
	std::vector< ExportData > exports = { };

	if( image )
	{
		const auto image_nt_headers = GetImageNtHeaders( image );

		if( image_nt_headers )
		{
			const auto image_data_directory = &image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];
			TRACE( "image_data_directory->Size = '0x%08X'", image_data_directory->Size );

			if( image_data_directory->VirtualAddress && image_data_directory->Size )
			{
				const auto image_export_directory = reinterpret_cast< const IMAGE_EXPORT_DIRECTORY* >( image + image_data_directory->VirtualAddress );

				if( image_export_directory->NumberOfNames )
				{
					const auto address_of_functions = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfFunctions );
					const auto address_of_names = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfNames );
					const auto address_of_name_ordinals = reinterpret_cast< const std::uint16_t* >( image + image_export_directory->AddressOfNameOrdinals );

					for( std::uint32_t index = 0; index < image_export_directory->NumberOfNames; index++ )
					{
						const auto export_name = reinterpret_cast< const char* >( image + address_of_names[ index ] );
						const auto export_ordinal = address_of_name_ordinals[ index ];
						const auto export_procedure = address_of_functions[ export_ordinal ];

						if( export_name[ 0 ] )
						{
							const auto export_name_hash = constant::Hash( export_name, true );
							exports.push_back( { export_name_hash, export_procedure } );
						}
					}
				}
			}
		}
	}

	return exports;
}

std::vector< SystemNumberData > GetSystemNumbers( std::uintptr_t image )
{
	std::vector< SystemNumberData > system_numbers = { };

	if( image )
	{
		const auto image_nt_headers = GetImageNtHeaders( image );

		if( image_nt_headers )
		{
			const auto image_data_directory = &image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];

			if( image_data_directory->VirtualAddress && image_data_directory->Size )
			{
				const auto image_export_directory = reinterpret_cast< const IMAGE_EXPORT_DIRECTORY* >( image + image_data_directory->VirtualAddress );

				if( image_export_directory->NumberOfNames )
				{
					const auto address_of_functions = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfFunctions );
					const auto address_of_names = reinterpret_cast< const std::uint32_t* >( image + image_export_directory->AddressOfNames );
					const auto address_of_name_ordinals = reinterpret_cast< const std::uint16_t* >( image + image_export_directory->AddressOfNameOrdinals );

					for( std::uint32_t index = 0; index < image_export_directory->NumberOfNames; index++ )
					{
						const auto export_name = reinterpret_cast< const char* >( image + address_of_names[ index ] );
						const auto export_ordinal = address_of_name_ordinals[ index ];
						const auto export_procedure = address_of_functions[ export_ordinal ];

						if( export_name[ 0 ] )
						{
							const auto export_number = GetSystemNumber( image + export_procedure );

							if( export_number != BadSystemNumber )
							{
								const auto export_name_hash = constant::Hash( export_name, true );
								system_numbers.push_back( { export_name_hash, export_number } );
							}
						}
					}
				}
			}
		}
	}

	return system_numbers;
}

io::MemoryBlock Allocate( std::size_t size, std::uint32_t protection = PAGE_READWRITE )
{
	io::MemoryBlock block = { };

	block.m_process_id = GetCurrentProcessId();
	block.m_size = size;
	block.m_protection = protection;
	block.m_address = memory::ToAddress( VirtualAlloc( nullptr, block.m_size, MEM_COMMIT | MEM_RESERVE, block.m_protection ) );
	
	return block;
}

void Free( const io::MemoryBlock& block )
{
	if( block.IsValid() )
	{
		VirtualFree( memory::ToPointer( block.m_address ), 0, MEM_RELEASE );
	}
}

template< typename Type >
void Write( const io::MemoryBlock& block, const Type* data, std::size_t size = sizeof( Type ), std::intptr_t offset = 0 )
{
	std::memcpy( memory::ToPointer( block.m_address + offset ), data, size );
}

void PrepareMapData( core::MapData* map_data )
{
	if( memory::IsAddressValid( map_data ) )
	{
		map_data->m_signature = core::MapData::Signature;
		map_data->m_time_expired.QuadPart = g_time_expire.QuadPart;

		const auto native = GetImage( L"ntdll.dll" );

		if( native )
		{
#define LOAD( name )	map_data->name = GetExport( native, #name )

			LOAD( RtlAddFunctionTable );
			LOAD( RtlDeleteFunctionTable );

			LOAD( RtlAllocateHeap );
			LOAD( RtlReAllocateHeap );
			LOAD( RtlFreeHeap );

			LOAD( RtlQueryPerformanceCounter );
			LOAD( RtlQueryPerformanceFrequency );

			LOAD( RtlInitializeCriticalSection );
			LOAD( RtlInitializeCriticalSectionEx );
			LOAD( RtlEnterCriticalSection );
			LOAD( RtlTryEnterCriticalSection );
			LOAD( RtlLeaveCriticalSection );
			LOAD( RtlDeleteCriticalSection );

			LOAD( RtlInitializeConditionVariable );
			LOAD( RtlSleepConditionVariableCS );
			LOAD( RtlWakeConditionVariable );
			LOAD( RtlWakeAllConditionVariable );

			map_data->__C_specific_handler = GetExport( native, "__C_specific_handler" );

#undef LOAD
		}
		else
		{
			throw std::runtime_error( __FUNCTION__ ": GetImage( 'ntdll.dll' ) error! (" + std::to_string( GetLastError() ) + ")" );
		}

		const auto kernel32 = GetImage( L"kernel32.dll" );

		if( kernel32 )
		{
			map_data->OutputDebugStringA = GetExport( kernel32, "OutputDebugStringA" );
			map_data->OutputDebugStringW = GetExport( kernel32, "OutputDebugStringW" );
		}
		else
		{
			throw std::runtime_error( __FUNCTION__ ": GetImage( 'kernel32.dll' ) error! (" + std::to_string( GetLastError() ) + ")" );
		}

		const auto msvcrt = GetImage( L"msvcrt.dll" );

		if( msvcrt )
		{
			map_data->malloc = GetExport( msvcrt, "malloc" );
			map_data->free = GetExport( msvcrt, "free" );

			map_data->memchr = GetExport( msvcrt, "memchr" );
			map_data->memcmp = GetExport( msvcrt, "memcmp" );
			map_data->memcpy = GetExport( msvcrt, "memcpy" );
			map_data->memmove = GetExport( msvcrt, "memmove" );
			map_data->memset = GetExport( msvcrt, "memset" );

			map_data->qsort = GetExport( msvcrt, "qsort" );
			map_data->strstr = GetExport( msvcrt, "strstr" );
			
			map_data->_vsnprintf_l = GetExport( msvcrt, "_vsnprintf_l" );
			map_data->_vsnwprintf_l = GetExport( msvcrt, "_vsnwprintf_l" );

			map_data->fabs = GetExport( msvcrt, "fabs" );
			map_data->fmod = GetExport( msvcrt, "fmod" );
			map_data->pow = GetExport( msvcrt, "pow" );
			map_data->atan2 = GetExport( msvcrt, "atan2" );
			map_data->ceil = GetExport( msvcrt, "ceil" );
			map_data->floor = GetExport( msvcrt, "floor" );
			map_data->atol = GetExport( msvcrt, "atol" );
			map_data->atof = GetExport( msvcrt, "atof" );

			map_data->sscanf = GetExport( msvcrt, "sscanf" );
		}
		else
		{
			throw std::runtime_error( __FUNCTION__ ": GetImage( 'msvcrt.dll' ) error! (" + std::to_string( GetLastError() ) + ")" );
		}

		if( native )
		{
			map_data->NtClose = GetSystemNumber( native, "NtClose" );
			map_data->NtDuplicateObject = GetSystemNumber( native, "NtDuplicateObject" );
			map_data->NtMakeTemporaryObject = GetSystemNumber( native, "NtMakeTemporaryObject" );
			map_data->NtQueryObject = GetSystemNumber( native, "NtQueryObject" );
			map_data->NtSetInformationObject = GetSystemNumber( native, "NtSetInformationObject" );
			map_data->NtSignalAndWaitForSingleObject = GetSystemNumber( native, "NtSignalAndWaitForSingleObject" );
			map_data->NtWaitForMultipleObjects = GetSystemNumber( native, "NtWaitForMultipleObjects" );
			map_data->NtWaitForSingleObject = GetSystemNumber( native, "NtWaitForSingleObject" );

			map_data->NtClearEvent = GetSystemNumber( native, "NtClearEvent" );
			map_data->NtCreateEvent = GetSystemNumber( native, "NtCreateEvent" );
			map_data->NtOpenEvent = GetSystemNumber( native, "NtOpenEvent" );
			map_data->NtPulseEvent = GetSystemNumber( native, "NtPulseEvent" );
			map_data->NtResetEvent = GetSystemNumber( native, "NtResetEvent" );
			map_data->NtSetEvent = GetSystemNumber( native, "NtSetEvent" );

			map_data->NtCreateFile = GetSystemNumber( native, "NtCreateFile" );
			map_data->NtReadFile = GetSystemNumber( native, "NtReadFile" );
			map_data->NtWriteFile = GetSystemNumber( native, "NtWriteFile" );
			map_data->NtLockFile = GetSystemNumber( native, "NtLockFile" );
			map_data->NtUnlockFile = GetSystemNumber( native, "NtUnlockFile" );
			map_data->NtDeleteFile = GetSystemNumber( native, "NtDeleteFile" );
			map_data->NtQueryInformationFile = GetSystemNumber( native, "NtQueryInformationFile" );
			map_data->NtQueryDirectoryFile = GetSystemNumber( native, "NtQueryDirectoryFile" );

			map_data->NtAllocateVirtualMemory = GetSystemNumber( native, "NtAllocateVirtualMemory" );
			map_data->NtFreeVirtualMemory = GetSystemNumber( native, "NtFreeVirtualMemory" );
			map_data->NtLockVirtualMemory = GetSystemNumber( native, "NtLockVirtualMemory" );
			map_data->NtUnlockVirtualMemory = GetSystemNumber( native, "NtUnlockVirtualMemory" );
			map_data->NtQueryVirtualMemory = GetSystemNumber( native, "NtQueryVirtualMemory" );
			map_data->NtProtectVirtualMemory = GetSystemNumber( native, "NtProtectVirtualMemory" );
			map_data->NtReadVirtualMemory = GetSystemNumber( native, "NtReadVirtualMemory" );
			map_data->NtWriteVirtualMemory = GetSystemNumber( native, "NtWriteVirtualMemory" );
		}

		const auto win32u = GetImage( L"win32u.dll" );

		if( win32u )
		{
			map_data->NtUserGetKeyNameText = GetSystemNumber( win32u, "NtUserGetKeyNameText" );
			map_data->NtUserGetKeyState = GetSystemNumber( win32u, "NtUserGetKeyState" );
			map_data->NtUserGetAsyncKeyState = GetSystemNumber( win32u, "NtUserGetAsyncKeyState" );
			map_data->NtUserMapVirtualKeyEx = GetSystemNumber( win32u, "NtUserMapVirtualKeyEx" );
		}
		else
		{
			throw std::runtime_error( __FUNCTION__ ": GetImage( 'win32u.dll' ) error! (" + std::to_string( GetLastError() ) + ")" );
		}
	}
}

std::uint64_t ExecuteProcedure( std::uint32_t process_id, std::uint32_t thread_id, std::uintptr_t procedure, std::uint64_t rcx, std::uint64_t rdx, std::uint64_t r8 )
{
	auto& driver = io::DriverControl::Instance();

	try
	{
		auto execute_size = sizeof( g_execute_code ) + sizeof( ExecuteData );
		auto execute_block = driver.Commit( execute_size );
		auto execute = std::make_unique< std::uint8_t[] >( execute_size );

		TRACE( "execute_block = '0x%016llX'", execute_block.m_address );

		// 
		// copy hook stub
		// 
		std::memcpy( &execute[ 0 ], g_execute_code, sizeof( g_execute_code ) );

		// 
		// copy execute data address
		// 
		auto execute_data_block = execute_block.m_address + sizeof( g_execute_code );
		std::memcpy( &execute[ 6 ], &execute_data_block, sizeof( execute_data_block ) );

		// 
		// get execute data pointer
		// 
		auto execute_data = reinterpret_cast< ExecuteData* >( execute.get() + sizeof( g_execute_code ) );

		execute_data->m_status = StatusExecuteWait;
		execute_data->m_procedure = procedure;
		execute_data->m_rcx = rcx;
		execute_data->m_rdx = rdx;
		execute_data->m_r8 = r8;

		execute_block.Write( execute.get(), execute_size );

		const auto hook = SetWindowsHookExW( WH_GETMESSAGE, reinterpret_cast< HOOKPROC >( execute_block.m_address ), GetModuleHandleW( L"ntdll.dll" ), thread_id );

		if( hook )
		{
			auto process_quit = false;

			while( execute_data->m_status != StatusExecuteLeave )
			{
				// const auto window = FindWindowW( L"UnrealWindow", nullptr );
				std::vector< ProcessRecord > process_array = { };
				if( GetProcessIdByName( L"notepad.exe", process_array ) )
				{
					if( process_array[ 0 ].m_process_id != process_id )
					{
						process_quit = true;
						break;
					}
				}
				else
				{
					process_quit = true;
					break;
				}

				if( !PostThreadMessageW( thread_id, WM_NULL, 0, 0 ) )
				{
					TRACE( "%s: PostThreadMessageW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
				}

				execute_block.Read( execute_data, sizeof( ExecuteData ), sizeof( g_execute_code ) );

				TRACE( "execute_data: m_status = '0x%08X', m_rax = '0x%016llX'", execute_data->m_status, execute_data->m_rax );

				if( execute_data->m_status == StatusExecuteLeave )
				{
					break;
				}

				std::this_thread::sleep_for( 250ms );
			}

			if( !process_quit )
			{
				if( !UnhookWindowsHookEx( hook ) )
				{
					TRACE( "%s: UnhookWindowsHookEx( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
				}

				driver.Free( execute_block );

				return execute_data->m_rax;
			}
		}
		else
		{
			TRACE( "%s: SetWindowsHookExW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		}
	}
	catch( const std::exception& exception )
	{
		TRACE( "%s: Exception: %s", __FUNCTION__, exception.what() );
	}

	return 0;
}

bool ExecuteImageEntry( std::uint32_t thread_id, std::uintptr_t image, const IMAGE_NT_HEADERS* image_nt_headers )
{
	auto& driver = io::DriverControl::Instance();

	try
	{
		auto execute_size = sizeof( g_execute_code ) + sizeof( ExecuteData );
		auto execute_block = driver.Commit( PAGE_SIZE, PAGE_READWRITE );
		auto execute = std::make_unique< std::uint8_t[] >( execute_size );

		// 
		// copy hook stub
		// 
		std::memcpy( execute.get(), g_execute_code, sizeof( g_execute_code ) );
		
		// 
		// copy execute data address
		// 
		auto execute_data_block = execute_block.m_address + sizeof( g_execute_code );		
		std::memcpy( &execute[ 28 ], &execute_data_block, sizeof( execute_data_block ) );

		// 
		// get execute data pointer
		// 
		auto execute_data = reinterpret_cast< ExecuteData* >( execute.get() + sizeof( g_execute_code ) );

		auto map_data_size = sizeof( core::MapData );
		auto map_data_block = driver.Commit( PAGE_SIZE, PAGE_READWRITE );
		
		core::MapData map_data =
		{
			core::MapData::Signature,
			image,
			static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage ),
		};

		PrepareMapData( &map_data );

		map_data_block.Write( &map_data );

		execute_data->m_status = StatusExecuteWait;
		execute_data->m_procedure = image + image_nt_headers->OptionalHeader.AddressOfEntryPoint;
		execute_data->m_rcx = image;
		execute_data->m_rdx = DLL_PROCESS_ATTACH;
		execute_data->m_r8 = map_data_block.m_address;

		execute_block.Write( execute.get(), execute_size );

		const auto nt = GetModuleHandleW( L"ntdll.dll" );
		const auto hook = SetWindowsHookExW( WH_GETMESSAGE, HOOKPROC( execute_block.m_address ), nt, thread_id );
		
		if( hook )
		{
			auto process_quit = false;

			while( execute_data->m_status != StatusExecuteLeave )
			{
				const auto window = FindWindowW( cx_game_class_name, nullptr );

				if( !window )
				{
					process_quit = true;
					break;
				}

				if( !PostThreadMessageW( thread_id, WM_NULL, 0, 0 ) )
				{
					TRACE( "%s: PostThreadMessageA( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
				}

				execute_block.Read( execute_data, sizeof( ExecuteData ), sizeof( g_execute_code ) );

				TRACE( "execute_data: m_status = '0x%08X', m_rax = '0x%016llX'", execute_data->m_status, execute_data->m_rax );

				if( execute_data->m_status == StatusExecuteLeave )
				{
					break;
				}

				std::this_thread::sleep_for( 250ms );
			}

			if( !process_quit )
			{
				if( !UnhookWindowsHookEx( hook ) )
				{
					TRACE( "%s: UnhookWindowsHookEx( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
				}

				// driver.Free( map_data_block );
				// driver.Free( execute_block );

				return true;
			}
		}
		else
		{
			TRACE( "%s: SetWindowsHookExW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		}
	}
	catch( const std::exception& exception )
	{
		TRACE( "%s: Exception: %s", __FUNCTION__, exception.what() );
	}

	return false;
}

const IMAGE_DOS_HEADER* get_image_dos_header( std::uintptr_t image )
{
	if( image == 0 )
	{
		return nullptr;
	}

	const auto image_dos_header = reinterpret_cast< const IMAGE_DOS_HEADER* >( image );

	if( image_dos_header->e_magic != IMAGE_DOS_SIGNATURE )
	{
		return nullptr;
	}

	return image_dos_header;
}

const IMAGE_NT_HEADERS* get_image_nt_headers( std::uintptr_t image )
{
	const auto image_dos_header = get_image_dos_header( image );

	if( image_dos_header == nullptr )
	{
		return nullptr;
	}

	const auto image_nt_headers = reinterpret_cast< const IMAGE_NT_HEADERS* >( image + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != IMAGE_NT_SIGNATURE )
	{
		return nullptr;
	}

	return image_nt_headers;
}

bool MapImage( const std::uint8_t* image_data, std::size_t image_size )
{
	const auto image_nt_headers = get_image_nt_headers( std::uintptr_t( image_data ) );

	if( image_nt_headers == nullptr )
	{
		TRACE( "%s: Not a valid nt image!", __FUNCTION__ );
		return false;
	}

	const auto image_map_size = static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage );

	if( image_map_size == 0 )
	{
		TRACE( "%s: SizeOfImage is not valid!", __FUNCTION__ );
		return false;
	}

	// 
	// create mapped image
	// 
	const auto image_map_data = ( std::uint8_t* )VirtualAlloc( nullptr, image_map_size, MEM_COMMIT, PAGE_READWRITE );

	if( image_map_data == 0 )
	{
		TRACE( "%s: VirtualAlloc( nullptr, '0x%016llX', MEM_COMMIT, PAGE_READWRITE ) error! (0x%08X)", __FUNCTION__, image_map_size, GetLastError() );
		return false;
	}
	
	// 
	// reset mapped data
	// 
	std::memset( image_map_data, 0, image_map_size );

	// 
	// copy image headers
	// 
	std::memcpy( image_map_data, image_data, image_nt_headers->OptionalHeader.SizeOfHeaders );

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	// 
	// iterate and copy image sections
	// 
	for( std::uint16_t i = 0; i < image_nt_headers->FileHeader.NumberOfSections; i++ )
	{
		const auto image_section = &image_section_header[ i ];

		// 
		// make sure section is valid
		// 
		if( image_section->VirtualAddress != 0 &&
				image_section->PointerToRawData != 0 )
		{
			const auto section_map = image_map_data + image_section->VirtualAddress;
			const auto section_data = image_data + image_section->PointerToRawData;

			// 
			// copy individual section data
			// 
			std::memcpy( section_map, section_data, image_section->SizeOfRawData );
		}
	}

	struct RelocationEntry
	{
		std::uintptr_t m_address = 0;
		std::uint16_t* m_item = nullptr;
		std::uint32_t m_count = 0;
	};

	const auto image_relocation_directory = &image_nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ];

	if( image_relocation_directory->VirtualAddress != 0 &&
			image_relocation_directory->Size != 0 )
	{
		auto base_relocation = reinterpret_cast< IMAGE_BASE_RELOCATION* >( const_cast< std::uint8_t* >( image_data ) + image_relocation_directory->VirtualAddress );
		auto base_relocation_end = reinterpret_cast< std::uintptr_t >( base_relocation ) + image_relocation_directory->Size;

		while( base_relocation->VirtualAddress != 0 &&
					 base_relocation->SizeOfBlock != 0 &&
					 base_relocation->VirtualAddress < base_relocation_end )
		{
			const auto address = reinterpret_cast< std::uintptr_t >( image_data ) + base_relocation->VirtualAddress;
			const auto item = reinterpret_cast< std::uint16_t* >( reinterpret_cast< std::uintptr_t >( base_relocation ) + sizeof( IMAGE_BASE_RELOCATION ) );
			const auto count = base_relocation->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) / sizeof( std::uint16_t );

			// 
			// #TODO: relocate image by delta
			// 

			base_relocation = reinterpret_cast< IMAGE_BASE_RELOCATION* >( reinterpret_cast< std::uintptr_t >( base_relocation ) + base_relocation->SizeOfBlock );
		}
	}
}

bool MapUserImage( const std::uint8_t* library_data, std::size_t library_size )
{
	auto& driver = io::DriverControl::Instance();

	auto image_data = std::uintptr_t( library_data );

	if( !image_data )
	{
		TRACE( "%s: image_data is not valid!", __FUNCTION__ );
		return false;
	}

	// 
	// get image pe headers
	// 
	auto image_nt_headers = GetImageNtHeaders( image_data );

	if( !image_nt_headers )
	{
		TRACE( "%s: image_nt_headers is not valid!", __FUNCTION__ );
		return false;
	}

	// 
	// get image virtual size from pe headers
	// 
	auto image_size = static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage );

	if( !image_size )
	{
		TRACE( "%s: image_size is not valid!", __FUNCTION__ );
		return false;
	}

	// 
	// allocate virtual image
	// 
	auto image_mapped = VirtualAlloc( nullptr, image_size, MEM_COMMIT, PAGE_READWRITE );
	auto image = std::uintptr_t( image_mapped );

	// 
	// copy image headers
	// 
	if( !MapImageHeader( image, image_data ) )
	{
		TRACE( "%s: MapImageHeader( ... ) error!", __FUNCTION__ );
		return false;
	}

	// 
	// resolve image imports
	// 
	if( !MapImageImportDescriptor( image ) )
	{
		TRACE( "%s: MapImageImportDescriptor( ... ) error!", __FUNCTION__ );
		return false;
	}

	// 
	// copy image sections
	// 
	if( !MapImageSection( image, image_data ) )
	{
		TRACE( "%s: MapImageSection( ... ) error!", __FUNCTION__ );
		return false;
	}

	// 
	// allocate memory block to map image to
	// 
	auto image_block = driver.Commit( image_size, PAGE_READWRITE );

	// 
	// make sure it is good to use
	// 
	if( !image_block.IsValid() )
	{
		TRACE( "%s: control.Commit( 0x%016llX ) error!", __FUNCTION__, image_size );
		return false;
	}

	TRACE( "image_block.m_address: 0x%016llX", image_block.m_address );
	TRACE( "image_block.m_size: 0x%016llX", image_block.m_size );
	
	// 
	// relocate image by delta
	// 
	if( !MapImageRelocation( image_block.m_address, image, image_nt_headers ) )
	{
		TRACE( "%s: MapImageRelocation( ... ) error!", __FUNCTION__ );
		return false;
	}

	// 
	// write image data to process
	// 
	if( !driver.WriteMemory( image_block.m_address, memory::ToPointer( image ), image_size ) )
	{
		TRACE( "%s: driver.WriteMemory( '0x%016llX', '0x%016llX', '0x%016llX' ) error!", image_block.m_address, image, image_size );
	}

	// 
	// use window hook to invoke entry point
	//
	if( !ExecuteImageEntry( driver.GetThreadId(), image_block.m_address, image_nt_headers ) )
	{
		TRACE( "%s: ExecuteProcedure( '0x%08X', '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, driver.GetThreadId(), image_block.m_address, memory::ToAddress( image_nt_headers ) );
		return false;
	}

	return true;
}

} // namespace atom