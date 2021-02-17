#include "loader.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../core/string.hpp"

#include "../resource/intel.hpp"

#include "../win32/console.hpp"
#include "../win32/handle_guard.hpp"

#include "../krypton/driver_map_context.hpp"

#include "../symbols/pdb_manager.hpp"
#include "../symbols/symbol_data.hpp"
#include "../symbols/symbol_loader.hpp"

constexpr std::uint32_t PoolTag = 'TDmM';

namespace atom::kernel
{

Loader::Loader( const std::wstring& path, const std::wstring& name )
	: m_sub_system( *this )
	, m_path( path )
	, m_object_name( name )
	, m_device_name( L"\\Device\\" + name )
{ }

Loader::~Loader()
{
	if( !Unload() )
	{
		TRACE( "%s: Couldn't unload 'Intel' driver on destructor!", __FUNCTION__ );
	}
}

bool Loader::Load()
{
	if( Connect() )
	{
		TRACE( "%s: 'Intel' driver is already loaded!", __FUNCTION__ );
		return true;
	}

	return Reload();
}

bool Loader::Reload()
{
	Unload();

	if( !LoadInternal() )
	{
		TRACE( "%s: LoadInternal() error!", __FUNCTION__ );
		return false;
	}

	return Connect();
}

bool Loader::Unload()
{
	Disconnect();

	return UnloadInternal();
}

bool Loader::CopyMemory( void* destination, const void* source, std::size_t size )
{
	COPY_MEMORY copy_memory = { };

	copy_memory.m_case_number = static_cast< std::uint64_t >( 51 );
	copy_memory.m_destination = reinterpret_cast< std::uint64_t >( destination );
	copy_memory.m_source = reinterpret_cast< std::uint64_t >( source );
	copy_memory.m_size = static_cast< std::uint64_t >( size );

	return DeviceControl( &copy_memory );
}

bool Loader::GetPhysicalAddress( std::uintptr_t virtual_address, std::uintptr_t* physical_address )
{
	GET_PHYSICAL_ADDRESS get_physical_address = { };

	get_physical_address.m_case_number = static_cast< std::uint64_t >( 37 );
	get_physical_address.m_virtual_address = virtual_address;

	if( !DeviceControl( &get_physical_address ) )
	{
		return false;
	}

	if( physical_address )
	{
		*physical_address = get_physical_address.m_physical_address;
	}

	return true;
}

bool Loader::MapIoSpace( std::uintptr_t physical_address, std::uint32_t size, std::uintptr_t* virtual_address )
{
	MAP_IO_SPACE map_io_space = { };

	map_io_space.m_case_number = static_cast< std::uint64_t >( 25 );
	map_io_space.m_physical_address = physical_address;
	map_io_space.m_size = size;

	if( !DeviceControl( &map_io_space ) )
	{
		return false;
	}

	if( virtual_address )
	{
		*virtual_address = map_io_space.m_virtual_address;
	}

	return true;
}

bool Loader::UnmapIoSpace( std::uintptr_t virtual_address, std::uint32_t size )
{
	UNMAP_IO_SPACE unmap_io_space = { };

	unmap_io_space.m_case_number = static_cast< std::uint64_t >( 26 );
	unmap_io_space.m_virtual_address = virtual_address;
	unmap_io_space.m_size = size;

	return DeviceControl( &unmap_io_space );
}

bool Loader::ReadMemory( std::uintptr_t address, void* data, std::size_t size )
{
	auto destination = data;
	auto source = reinterpret_cast< const void* >( address );

	return CopyMemory( destination, source, size );
}

bool Loader::WriteMemory( std::uintptr_t address, const void* data, std::size_t size )
{
	auto destination = reinterpret_cast< void* >( address );
	auto source = data;

	return CopyMemory( destination, source, size );
}

bool Loader::WriteMemoryReadOnly( std::uintptr_t address, const void* data, std::size_t size )
{
	std::uintptr_t physical_address = 0;

	if( !GetPhysicalAddress( address, &physical_address ) )
	{
		return false;
	}

	bool result = false;

	std::uintptr_t virtual_address = 0;

	// 
	// map writeable memory
	// 
	if( MapIoSpace( physical_address, static_cast< std::uint32_t >( size ), &virtual_address ) )
	{
		// 
		// write data
		// 
		result = WriteMemory( virtual_address, data, size );

		// 
		// unmap writeable memory
		// 
		UnmapIoSpace( virtual_address, static_cast< std::uint32_t >( size ) );
	}

	return result;
}

std::uintptr_t Loader::GetSystemImage( const std::string& name )
{
	PVOID device_drivers[ 1024 ] = { };
	DWORD device_drivers_size = 0;

	K32EnumDeviceDrivers( device_drivers, sizeof( device_drivers ), &device_drivers_size );

	DWORD device_drivers_count = device_drivers_size / sizeof( PVOID );

	for( DWORD i = 0; i < device_drivers_count; i++ )
	{
		const auto device_driver = device_drivers[ i ];

		if( !device_driver )
			continue;

		char device_driver_name[ MAX_PATH ] = { };
		K32GetDeviceDriverBaseNameA( device_driver, device_driver_name, MAX_PATH );

		if( core::ToLower( device_driver_name ) == core::ToLower( name ) )
		{
			return std::uintptr_t( device_driver );
		}
	}

	return 0;
}

std::uintptr_t Loader::GetSystemImage( const std::wstring& name )
{
	PVOID device_drivers[ 1024 ] = { };
	DWORD device_drivers_size = 0;

	K32EnumDeviceDrivers( device_drivers, sizeof( device_drivers ), &device_drivers_size );

	DWORD device_drivers_count = device_drivers_size / sizeof( PVOID );

	for( DWORD i = 0; i < device_drivers_count; i++ )
	{
		const auto device_driver = device_drivers[ i ];

		if( !device_driver )
			continue;

		wchar_t device_driver_name[ MAX_PATH ] = { };
		K32GetDeviceDriverBaseNameW( device_driver, device_driver_name, MAX_PATH );

		if( core::ToLower( device_driver_name ) == core::ToLower( name ) )
		{
			return std::uintptr_t( device_driver );
		}
	}

	return 0;
}

std::uintptr_t Loader::GetImageExport( std::uintptr_t image, const std::string& name )
{
	if( !image )
	{
		TRACE( "%s: image is 0!", __FUNCTION__ );
		return 0;
	}

	const auto image_dos_header = Read< IMAGE_DOS_HEADER >( image );

	if( image_dos_header.e_magic != IMAGE_DOS_SIGNATURE )
	{
		TRACE( "%s: Not a valid DOS image!", __FUNCTION__ );
		return 0;
	}

	const auto image_nt_headers = Read< IMAGE_NT_HEADERS >( image + image_dos_header.e_lfanew );

	if( image_nt_headers.Signature != IMAGE_NT_SIGNATURE )
	{
		TRACE( "%s: Not a valid NT image!", __FUNCTION__ );
		return 0;
	}

	const auto& directory_entry_export = image_nt_headers.OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ];

	if( !directory_entry_export.VirtualAddress || !directory_entry_export.Size )
	{
		TRACE( "%s: Not a valid image export directory!", __FUNCTION__ );
		return 0;
	}

	const auto image_export_directory_data = std::make_unique< std::uint8_t[] >( directory_entry_export.Size );

	if( !ReadMemory( image + directory_entry_export.VirtualAddress, image_export_directory_data.get(), directory_entry_export.Size ) )
	{
		TRACE( "%s: ReadMemory( 0x%016llX, 0x%08X ) error!", __FUNCTION__, image + directory_entry_export.VirtualAddress, directory_entry_export.Size );
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

	return 0;
}

bool Loader::InitializeUserCode()
{
	const auto image_native = std::uintptr_t( LoadLibraryW( L"ntdll.dll" ) );

	if( image_native )
	{
		m_user_procedure = std::uintptr_t( GetProcAddress( HMODULE( image_native ), "NtQueryInformationAtom" ) );

		if( m_user_procedure )
		{
			return true;
		}
	}

	m_image_gdi32 = std::uintptr_t( LoadLibraryW( L"gdi32full.dll" ) );

	if( !m_image_gdi32 )
	{
		m_image_gdi32 = std::uintptr_t( LoadLibraryW( L"gdi32.dll" ) );

		if( !m_image_gdi32 )
		{
			TRACE( "%s: LoadLibraryW( L\"gdi32.dll\" ) error! (0x%08X)", __FUNCTION__, GetLastError() );
			return false;
		}
	}

	const auto win32u = std::uintptr_t( LoadLibraryW( L"win32u.dll" ) );

	if( win32u )
	{
		m_user_procedure = std::uintptr_t( GetProcAddress( HMODULE( win32u ), "NtGdiGetCOPPCompatibleOPMInformation" ) );

		if( m_user_procedure )
		{
			m_windows_10 = true;
			return true;
		}
	}

	m_user_procedure = std::uintptr_t( GetProcAddress( HMODULE( m_image_gdi32 ), "NtGdiGetCOPPCompatibleOPMInformation" ) );

	if( !m_user_procedure )
	{
		m_user_procedure = std::uintptr_t( GetProcAddress( HMODULE( m_image_gdi32 ), "GetCOPPCompatibleOPMInformation" ) );

		if( !m_user_procedure )
		{
			TRACE( "%s: GetProcAddress( 0x%016llX, \"NtGdiGetCOPPCompatibleOPMInformation\" ) error! (0x%08X)", __FUNCTION__, m_image_gdi32, GetLastError() );
			return false;
		}

		m_windows_7 = true;
	}

	return true;
}

bool Loader::InitializeKernelCode()
{
	m_image_ntoskrnl = GetSystemImage( "ntoskrnl.exe" );

	/*m_image_win32k = GetSystemImage( "win32kfull.sys" );

	if( !m_image_win32k )
	{
		m_image_win32k = GetSystemImage( "win32k.sys" );

		if( !m_image_win32k )
		{
			HORIZON_LOG( "%s: GetSystemImage( \"win32k.sys\" ) error!", __FUNCTION__ );
			return false;
		}
	}

	auto kernel_procedure = GetImageExport( m_image_win32k, "NtGdiGetCOPPCompatibleOPMInformation" );

	if( !kernel_procedure )
	{
		m_image_win32k = GetSystemImage( "win32k.sys" );

		const auto w32p_service_array = GetImageExport( m_image_win32k, "W32pServiceTable" );

		if( !w32p_service_array )
		{
			TRACE( "%s: Couldn't find 'W32pServiceTable' export!", __FUNCTION__ );
			return false;
		}

		const auto syscall = *reinterpret_cast< std::uint32_t* >( m_user_procedure + 4 );
		const auto kernel_procedure_address = w32p_service_array + ( static_cast< std::size_t >( syscall ) - PAGE_SIZE ) * 8;

		TRACE( "'NtGdiGetCOPPCompatibleOPMInformation' syscall = '0x%08X'", syscall );
		
		if( !ReadMemory( kernel_procedure_address, &kernel_procedure, sizeof( kernel_procedure ) ) )
		{
			TRACE( "%s: Couldn't read 'NtGdiGetCOPPCompatibleOPMInformation' address!", __FUNCTION__ );
			return false;
		}

		TRACE( "NtGdiGetCOPPCompatibleOPMInformation @ '0x%016llX'", kernel_procedure );

		m_windows_7 = true;
	}*/

	m_kernel_procedure_pointer = GetImageExport( m_image_ntoskrnl, "NtQueryInformationAtom" );

	if( !ReadMemory( m_kernel_procedure_pointer, m_kernel_procedure_code.data(), m_kernel_procedure_code.size() ) )
	{
		HORIZON_LOG( "%s: ReadMemory( 0x%016llX, 0x%016llX, 0x%016llX ) error!", __FUNCTION__, m_kernel_procedure_pointer, std::uintptr_t( m_kernel_procedure_code.data() ), m_kernel_procedure_code.size() );
		return false;
	}

	return ( m_kernel_procedure_code[ 0 ] );
}

NtSystem& Loader::GetNtSystem()
{
	return m_sub_system;
}

const NtSystem& Loader::GetNtSystem() const
{
	return m_sub_system;
}

PIMAGE_NT_HEADERS Loader::GetImageNtHeaders( std::uintptr_t image )
{
	if( !image )
		return nullptr;

	const auto image_dos_header = PIMAGE_DOS_HEADER( image );

	if( image_dos_header->e_magic != IMAGE_DOS_SIGNATURE )
		return nullptr;

	const auto image_nt_headers = PIMAGE_NT_HEADERS( image + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != IMAGE_NT_SIGNATURE )
		return nullptr;

	return image_nt_headers;
}

bool Loader::MapHeader( std::uintptr_t image, std::uintptr_t data )
{
	auto nt_headers = GetImageNtHeaders( data );

	if( !nt_headers )
		return false;

	HORIZON_LOG( "Mapping image header from 0x%016llX to 0x%016llX with size 0x%08X", data, image, nt_headers->OptionalHeader.SizeOfHeaders );
	std::memcpy( PVOID( image ), PVOID( data ), PAGE_SIZE );
	return true;
}

bool Loader::MapSection( std::uintptr_t image, std::uintptr_t data )
{
	auto nt_headers = GetImageNtHeaders( image );

	if( !nt_headers )
		return false;

	auto section_header = IMAGE_FIRST_SECTION( nt_headers );

	if( !section_header )
		return false;

	for( auto i = 0; i < nt_headers->FileHeader.NumberOfSections; i++ )
	{
		auto section = section_header + i;

		if( !section )
			continue;

		char section_name[ 9 ] = { };
		std::memcpy( section_name, section->Name, sizeof( section->Name ) );

		HORIZON_LOG( "Mapping \"%s\" section from 0x%016llX to 0x%016llX with size 0x%08X", section_name, data + section->PointerToRawData, image + section->VirtualAddress, section->SizeOfRawData );
		std::memcpy( PVOID( image + section->VirtualAddress ), PVOID( data + section->PointerToRawData ), std::size_t( section->SizeOfRawData ) );
	}

	return true;
}

struct RelocationRecord
{
	std::uintptr_t m_address = 0;
	std::uint16_t* m_item = nullptr;
	std::uint32_t m_count = 0;
};

bool Loader::MapRelocation( std::uintptr_t image )
{
	auto nt_headers = GetImageNtHeaders( image );

	if( !nt_headers )
		return false;

	auto data_directory = nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BASERELOC ];

	if( !data_directory.VirtualAddress || !data_directory.Size )
		return true;

	auto base_relocation = PIMAGE_BASE_RELOCATION( image + data_directory.VirtualAddress );
	auto base_relocation_end = std::uintptr_t( base_relocation ) + data_directory.Size;

	while( base_relocation && base_relocation->VirtualAddress && base_relocation->SizeOfBlock && base_relocation->VirtualAddress < base_relocation_end )
	{
		RelocationRecord record =
		{
			image + base_relocation->VirtualAddress,
			reinterpret_cast< std::uint16_t* >( std::uintptr_t( base_relocation ) + sizeof( IMAGE_BASE_RELOCATION ) ),
			static_cast< std::uint32_t >( ( base_relocation->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( std::uint16_t ) ),
		};

		for( auto i = 0u; i < record.m_count; i++ )
		{
			const auto type = static_cast< std::uint16_t >( record.m_item[ i ] >> 12 );
			const auto offset = static_cast< std::uint16_t >( record.m_item[ i ] & 0xFFF );

			if( type == IMAGE_REL_BASED_DIR64 ||
					type == IMAGE_REL_BASED_HIGHLOW )
			{
				HORIZON_LOG( "Mapping relocation: 0x%016llX, 0x%04X, 0x%04X, 0x%016llX", record.m_address, type, offset, image - nt_headers->OptionalHeader.ImageBase );
				*reinterpret_cast< std::uintptr_t* >( record.m_address + offset ) += image - nt_headers->OptionalHeader.ImageBase;
			}
		}

		base_relocation = PIMAGE_BASE_RELOCATION( std::uintptr_t( base_relocation ) + base_relocation->SizeOfBlock );
	}

	return true;
}

bool Loader::MapImportDescriptor( std::uintptr_t image )
{
	auto nt_headers = GetImageNtHeaders( image );

	if( !nt_headers )
		return false;

	auto data_directory = nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ];

	if( !data_directory.VirtualAddress || !data_directory.Size )
		return true;

	auto import_descriptor = PIMAGE_IMPORT_DESCRIPTOR( image + data_directory.VirtualAddress );

	while( import_descriptor && import_descriptor->Name )
	{
		auto import_descriptor_name = PCHAR( image + import_descriptor->Name );
		auto import_descriptor_image = GetSystemImage( import_descriptor_name );

		if( import_descriptor_image )
		{
			auto thunk_data = PIMAGE_THUNK_DATA( image + import_descriptor->FirstThunk );

			while( thunk_data && thunk_data->u1.AddressOfData )
			{
				if( !( thunk_data->u1.Ordinal & IMAGE_ORDINAL_FLAG64 ) )
				{
					auto import_by_name = PIMAGE_IMPORT_BY_NAME( image + thunk_data->u1.AddressOfData );
					auto import_procedure = GetImageExport( import_descriptor_image, import_by_name->Name );

					if( import_procedure )
					{
						HORIZON_LOG( "Mapping \"%s\" (0x%016llX) import from \"%s\" (0x%016llX)", import_by_name->Name, import_procedure, import_descriptor_name, import_descriptor_image );
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

bool Loader::MapImage( std::uintptr_t image, std::uintptr_t base )
{
	auto nt_headers = GetImageNtHeaders( image );

	if( !nt_headers )
		return false;

	if( !WriteMemory( base, PVOID( image ), nt_headers->OptionalHeader.SizeOfImage ) )
		return false;

	return true;
}

bool Loader::EraseSection( std::uintptr_t image, std::uintptr_t base )
{
	auto nt_headers = GetImageNtHeaders( image );

	if( !nt_headers )
		return false;

	auto section_header = IMAGE_FIRST_SECTION( nt_headers );

	for( auto i = 0; i < nt_headers->FileHeader.NumberOfSections; i++ )
	{
		auto section = section_header + i;

		if( !section )
			continue;

		if( section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE )
		{
			HORIZON_LOG( "Erasing \"%s\" section at 0x%016llX with size 0x%08X", section->Name, base + section->VirtualAddress, section->SizeOfRawData );
			m_sub_system.memset( PVOID( base ), 0, std::size_t( nt_headers->OptionalHeader.SizeOfHeaders ) );
		}
	}

	HORIZON_LOG( "Erasing image header at 0x%016llX with size 0x%08X", base, nt_headers->OptionalHeader.SizeOfHeaders );	
	m_sub_system.memset( PVOID( base ), 0, std::size_t( nt_headers->OptionalHeader.SizeOfHeaders ) );

	return true;
}

bool Loader::ExecuteImageEntry( std::uintptr_t image, std::uintptr_t base )
{
	auto image_nt_headers = GetImageNtHeaders( image );

	if( !image_nt_headers )
	{

		return false;
	}

	auto address_of_entry_point = image_nt_headers->OptionalHeader.AddressOfEntryPoint;

	if( !address_of_entry_point )
	{

		return false;
	}

	auto entry_point = base + address_of_entry_point;

	auto remote_map_data = m_sub_system.Allocate( PAGE_SIZE );
	TRACE( "remote_map_data = '0x%016llX'", remote_map_data );

	if( remote_map_data )
	{
		atom::SystemMapData map_data = { };

		map_data.m_signature = HASH( "SystemMapData" );
		map_data.m_base = base;
		map_data.m_size = static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage );

		TRACE( "SymbolLoader::Constructor() begin" );
		SymbolLoader loader = { };
		TRACE( "SymbolLoader::Constructor() end" );

		// 
		// load 'ntoskrnl.exe' symbols
		// 
		TRACE( "SymbolLoader::Load() begin" );
		auto status = loader.Load( map_data.m_symbols );
		TRACE( "SymbolLoader::Load() end = '0x%08X'", status );

		if( NT_SUCCESS( status ) )
		{
			const auto symbols_size = sizeof( map_data.m_symbols );
			const auto symbols_count = symbols_size / sizeof( std::uintptr_t );

			for( std::size_t index = 0; index < symbols_count; index++ )
			{
				auto& symbol = ( &map_data.m_symbols.PsLoadedModuleList )[ index ];
				symbol += m_image_ntoskrnl;
			}

			// 
			// write map data to system pool
			// 
			if( WriteMemory( remote_map_data, &map_data, sizeof( map_data ) ) )
			{
				if( g_use_debug_exception )
				{
					// 
					// execute 'RtlInsertInvertedFunctionTable' function to add exception support
					// 
					if( m_windows_7 )
					{
						const auto PsInvertedFunctionTable = Read< std::uintptr_t >( map_data.m_symbols.PsInvertedFunctionTable );
						TRACE( "PsInvertedFunctionTable = '0x%016llX'", PsInvertedFunctionTable );

						if( PsInvertedFunctionTable )
						{
							const auto RtlInsertInvertedFunctionTable = map_data.m_symbols.RtlInsertInvertedFunctionTable;
							TRACE( "RtlInsertInvertedFunctionTable = '0x%016llX'", RtlInsertInvertedFunctionTable );

							if( !Call< void >( RtlInsertInvertedFunctionTable, nullptr, PsInvertedFunctionTable, base, image_nt_headers->OptionalHeader.SizeOfImage ) )
							{
								TRACE( "%s: Call( '0x%016llX', nullptr, '0x%016llX', '0x%016llX', '0x%08X' ) error!", __FUNCTION__, RtlInsertInvertedFunctionTable, PsInvertedFunctionTable, base, image_nt_headers->OptionalHeader.SizeOfImage );
							}
						}
						else
						{
							TRACE( "%s: PsInvertedFunctionTable is not valid!", __FUNCTION__ );
						}
					}
					else
					{
						const auto RtlInsertInvertedFunctionTable = map_data.m_symbols.RtlInsertInvertedFunctionTable;
						TRACE( "RtlInsertInvertedFunctionTable = '0x%016llX'", RtlInsertInvertedFunctionTable );

						if( !Call< void >( RtlInsertInvertedFunctionTable, nullptr, base, image_nt_headers->OptionalHeader.SizeOfImage ) )
						{
							TRACE( "%s: Call( '0x%016llX', nullptr, '0x%016llX', '0x%08X' ) error!", __FUNCTION__, RtlInsertInvertedFunctionTable, base, image_nt_headers->OptionalHeader.SizeOfImage );
						}
					}
				}

				// 
				// execute 'DriverEntry' function
				// 
				if( Call( entry_point, &status, remote_map_data, nullptr ) )
				{
					// 
					// show call parameters and return status
					// 
					TRACE( "DriverEntry( '0x%016llX', nullptr ) = '0x%08X'", remote_map_data, status );

					// 
					// free map data system pool
					// 
					m_sub_system.Free( remote_map_data );
					return true;
				}
				else
				{
					TRACE( "%s: Call( '0x%016llX', '0x%016llX', '0x%016llX', nullptr ) error!", __FUNCTION__, entry_point, memory::ToAddress( &status ), remote_map_data );
				}
			}
			else
			{
				TRACE( "%s: WriteMemory( '0x%016llX', '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, remote_map_data, memory::ToAddress( &map_data ), sizeof( map_data ) );
			}
		}
		else
		{
			TRACE( "%s: loader.Load( ... ) error! (0x%08X)", __FUNCTION__, status );
		}

		m_sub_system.Free( remote_map_data );
	}
	else
	{
		TRACE( "%s: m_sub_system.Allocate( '0x%016llX' ) error!", __FUNCTION__, PAGE_SIZE );
	}

	return false;
}

bool Loader::Map( const std::uint8_t* file_data, std::size_t file_size )
{
	auto file_base = memory::ToAddress( file_data );
	TRACE( "file_base = '0x%016llX'", file_base );

	auto file_nt_headers = GetImageNtHeaders( file_base );
	TRACE( "file_nt_headers = '0x%016llX'", memory::ToAddress( file_nt_headers ) );

	if( !file_nt_headers )
	{
		TRACE( "%s: GetImageNtHeaders( '0x%016llX' ) error!", __FUNCTION__, file_base );
		return false;
	}

	auto image_size = static_cast< std::size_t >( file_nt_headers->OptionalHeader.SizeOfImage );
	TRACE( "image_size = '0x%016llX' (%llu)", image_size, image_size );
	
	if( !image_size )
	{
		TRACE( "%s: file_nt_headers->OptionalHeader.SizeOfImage is 0!", __FUNCTION__ );
		return false;
	}

	auto image_data = std::make_unique< std::uint8_t[] >( image_size );

	auto image_base = memory::ToAddress( image_data.get() );
	TRACE( "image_base = '0x%016llX'", image_base );

	if( !MapHeader( image_base, file_base ) )
	{
		TRACE( "%s: MapHeader( '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, image_base, file_base );
		return false;
	}

	if( !MapSection( image_base, file_base ) )
	{
		TRACE( "%s: MapSection( '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, image_base, file_base );
		return false;
	}

	if( !MapRelocation( image_base ) )
	{
		TRACE( "%s: MapRelocation( '0x%016llX' ) error!", __FUNCTION__, image_base );
		return false;
	}

	if( !MapImportDescriptor( image_base ) )
	{
		TRACE( "%s: MapImportDescriptor( '0x%016llX' ) error!", __FUNCTION__, image_base );
		return false;
	}

	auto remote_image_base = m_sub_system.Allocate( image_size );

	if( remote_image_base )
	{
		TRACE( "remote_image_base = '0x%016llX'", remote_image_base );

		if( MapImage( image_base, remote_image_base ) )
		{
			if( ExecuteImageEntry( image_base, remote_image_base ) )
			{
				return true;
			}
			else
			{
				TRACE( "%s: ExecuteImageEntry( '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, image_base, remote_image_base );
			}
		}
		else
		{
			TRACE( "%s: MapImage( '0x%016llX', '0x%016llX' ) error!", __FUNCTION__, image_base, remote_image_base );
		}

		m_sub_system.Free( remote_image_base );
	}
	else
	{
		TRACE( "%s: m_sub_system.Allocate( '0x%016llX' ) error!", __FUNCTION__, image_size );
	}

	return false;
}

bool Loader::Connect()
{
	if( m_device.IsValid() )
	{
		return true;
	}

	UNICODE_STRING device_name = { };
	win32::NtCall< void >( "RtlInitUnicodeString", &device_name, m_device_name.c_str() );

	OBJECT_ATTRIBUTES object_attributes =
	{
		sizeof( OBJECT_ATTRIBUTES ),
		nullptr, &device_name,
	};

	IO_STATUS_BLOCK io_status_block = { };

	const auto status = NtOpenFile( &m_device, FILE_GENERIC_READ, &object_attributes, &io_status_block, FILE_SHARE_READ, FILE_OPEN_IF );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: NtOpenFile( ... ) error! (0x%08X)", __FUNCTION__, status );
		return false;
	}

	return m_device.IsValid();
}

void Loader::Disconnect()
{
	m_device.Reset();
}

bool Loader::LoadInternal()
{
	if( !CreateRegistry() )
	{
		TRACE( "%s: CreateRegistry() error!", __FUNCTION__ );
		return false;
	}

	const auto registry_path = ( L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\"s + m_object_name );
	const auto result = win32::NtLoadDriver( registry_path );

	if( !DestroyRegistry() )
	{
		TRACE( "%s: DestroyRegistry() error!", __FUNCTION__ );
		return false;
	}

	return result;
}

bool Loader::UnloadInternal()
{
	if( !CreateRegistry() )
	{
		TRACE( "%s: CreateRegistry() error!", __FUNCTION__ );
		return false;
	}

	const auto registry_path = ( L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\"s + m_object_name );
	const auto result = win32::NtUnloadDriver( registry_path );

	if( !DestroyRegistry() )
	{
		TRACE( "%s: DestroyRegistry() error!", __FUNCTION__ );
		return false;
	}

	return result;
}

bool Loader::CreateRegistry()
{
	if( m_path.empty() )
	{
		TRACE( "%s: m_path is not valid!", __FUNCTION__ );
		return false;
	}

	win32::RegistryHandle root = { };	
	
	auto status = RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", REG_OPTION_RESERVED, KEY_ALL_ACCESS, &root );

	if( status != ERROR_SUCCESS )
	{
		TRACE( "%s: RegOpenKeyExW( HKEY_LOCAL_MACHINE, 'SYSTEM\\CurrentControlSet\\Services' ) error! (0x%08X)", __FUNCTION__, status );
		return false;
	}

	win32::RegistryHandle key = { };

	status = RegCreateKeyExW( root, m_object_name.c_str(), 0, nullptr, REG_OPTION_RESERVED, KEY_ALL_ACCESS, nullptr, &key, nullptr );

	if( status != ERROR_SUCCESS )
	{
		TRACE( "%s: RegCreateKeyExW( 'Nal' ) error! (0x%08X)", __FUNCTION__, status );
		return false;
	}

	const auto type = static_cast< DWORD >( 1 );
	const auto type_size = static_cast< DWORD >( sizeof( type ) );

	status = RegSetValueExW( key, L"Type", 0, REG_DWORD, PBYTE( &type ), type_size );

	if( status != ERROR_SUCCESS )
	{
		TRACE( "%s: RegSetValueExW( 'Type', '0x%08X', '0x%08X' ) error! (0x%08X)", __FUNCTION__, type, type_size, status );
		return false;
	}

	const auto image_path = L"\\??\\"s + m_path;
	const auto image_path_size = static_cast< DWORD >( ( image_path.length() + 1 ) * sizeof( wchar_t ) );

	status = RegSetValueExW( key, L"ImagePath", 0, REG_SZ, PBYTE( image_path.c_str() ), image_path_size );

	if( status != ERROR_SUCCESS )
	{
		HORIZON_LOG( "%s: RegSetValueExW( 'ImagePath', '%S', '0x%08X' ) error! (0x%08X)", __FUNCTION__, image_path.c_str(), image_path_size, status );
		return false;
	}

	return true;
}

bool Loader::DestroyRegistry()
{
	win32::RegistryHandle root = { };

	auto status = RegOpenKeyExW( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", REG_OPTION_RESERVED, KEY_ALL_ACCESS, &root );

	if( status != ERROR_SUCCESS )
	{
		TRACE( "%s: RegOpenKeyExW( HKEY_LOCAL_MACHINE, 'SYSTEM\\CurrentControlSet\\Services' ) error! (0x%08X)", __FUNCTION__, status );
		return false;
	}

	status = RegDeleteTreeW( root, m_object_name.c_str() );

	if( status == ERROR_FILE_NOT_FOUND )
	{
		// 
		// registry is fine ...
		// 
		return true;
	}

	if( status != ERROR_SUCCESS )
	{
		TRACE( "%s: RegDeleteTreeW( '%S' ) error! (0x%08X)", __FUNCTION__, m_object_name.c_str(), status );
		return false;
	}

	return true;
}

} // namespace atom::kernel