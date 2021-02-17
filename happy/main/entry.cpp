#define _CRT_SECURE_NO_WARNINGS

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../core/map_data.hpp"
#include "../core/timer.hpp"

#include "../io/packet.hpp"
#include "../io/driver_control.hpp"

#include "../kernel/loader.hpp"

#include "../memory/operation.hpp"

#include "../options/configuration.hpp"

#include "../resource/data_loader.hpp"
#include "../resource/doze.hpp"
#include "../resource/herby.hpp"
#include "../resource/intel.hpp"
#include "../resource/pubg.hpp"

#include "../win32/console.hpp"
#include "../win32/dialog.hpp"
#include "../win32/file.hpp"
#include "../win32/handle_guard.hpp"
#include "../win32/native.hpp"
#include "../win32/privilege.hpp"
#include "../win32/debug_trace.hpp"
#include "../win32/version.hpp"

#include "../win32/pe_image.hpp"

#include "device_drivers.hpp"
#include "dump.hpp"
#include "mmap.hpp"
#include "privileges.hpp"
#include "unloaded_drivers.hpp"

#include "../symbols/symbol_data.hpp"
#include "../symbols/symbol_loader.hpp"

#include <Windows.h>
#include <WinInet.h>
#include <codecvt>

using namespace std::chrono_literals;

constexpr auto g_wait_process_timeout = 5min;
constexpr auto g_wait_image_timeout = 1min;

#undef GetCurrentDirectory

HORIZON_IMPORT( "ntdll.lib" );
HORIZON_IMPORT( "dbghelp.lib" );
HORIZON_IMPORT( "wininet" );

extern "C"
{

	__declspec( dllimport ) NTSTATUS NTAPI NtCreateThreadEx( HANDLE* ThreadHandle,
																													 ACCESS_MASK DesiredAccess,
																													 void* ObjectAttributes,
																													 HANDLE ProcessHandle,
																													 LPTHREAD_START_ROUTINE StartAddress,
																													 void* Parameter,
																													 std::uint32_t Flags,
																													 std::size_t StackZeroBits,
																													 std::size_t SizeOfStackCommit,
																													 std::size_t SizeOfStackReserve,
																													 void* BytesBuffer );

};

// 
// constants
// 
namespace atom
{

constexpr auto cx_ntoskrnl_driver_name = L"ntoskrnl.exe";
constexpr auto cx_win32kbase_driver_name = L"win32kbase.sys";
constexpr auto cx_mrxsmb_driver_name = L"mrxsmb.sys";

constexpr auto cx_intel_driver_name = L"iqvm64e.sys";
constexpr auto cx_intel_device_name = L"Nal";
constexpr auto cx_intel_time_date_stamp = 0x5284EAC3;

} // namespace atom

// 
// drivers
// 
namespace atom
{

std::uintptr_t GetSystemImage( const wchar_t* const name );
std::uintptr_t GetSystemImage( std::uint64_t name_hash );

void TestDrivers()
{
	/*resource::DataLoader data_loader( cx_intel_driver_name );
	
	if( !data_loader.IsValid() )
	{
		TRACE( "%s: Couldn't initialize data loader!", __FUNCTION__ );
	}

	const auto path = data_loader.GetDriverPath();
	kernel::Loader driver( path, cx_intel_device_name );
	
	if( !driver.Load() )
	{
		TRACE( "%s: Couldn't load 'Intel' driver!", __FUNCTION__ );
	}

	if( !driver.InitializeUserCode() )
	{
		TRACE( "%s: Couldn't initialize user code!", __FUNCTION__ );
	}

	if( !driver.InitializeKernelCode() )
	{
		TRACE( "%s: Couldn't initialize kernel code!", __FUNCTION__ );
	}*/

	auto& driver = io::DriverControl::Instance();
	driver.SetProcessId( 4 );

	const auto image_base = GetSystemImage( HASH( L"BEDaisy.sys" ) );
	TRACE( "image_base @ '0x%016llX'", image_base );

	if( image_base )
	{
		const auto image_dos_header = driver.Read< IMAGE_DOS_HEADER >( image_base );

		if( image_dos_header.e_magic == IMAGE_DOS_SIGNATURE )
		{
			const auto image_nt_headers = driver.Read< IMAGE_NT_HEADERS >( image_base + image_dos_header.e_lfanew );

			if( image_nt_headers.Signature == IMAGE_NT_SIGNATURE )
			{
				const auto image_size = static_cast< std::size_t >( image_nt_headers.OptionalHeader.SizeOfImage );
				TRACE( "image_size @ '0x%016llX' (%llu)", image_size, image_size );

				if( image_size )
				{
					const auto image_data = std::make_unique< std::uint8_t[] >( image_size );

					if( driver.ReadMemory( image_base, image_data.get(), image_size ) )
					{
						auto image_file = fopen( "BEDaisy.sys", "w" );

						if( image_file )
						{
							const auto result = fwrite( image_data.get(), sizeof( std::uint8_t ), image_size, image_file );
							TRACE( "fwrite( ... ) result = '0x%016llX' (%llu)", result, result );

							fclose( image_file );
						}
						else
						{
							TRACE( "%s: Couldn't create driver image file!", __FUNCTION__ );
						}
					}
					else
					{
						TRACE( "%s: Couldn't read driver image!", __FUNCTION__ );
					}
				}
			}
			else
			{
				TRACE( "%s: image_nt_headers are not valid!", __FUNCTION__ );
			}
		}
		else
		{
			TRACE( "%s: image_dos_header is not valid!", __FUNCTION__ );
		}
	}
	else
	{
		TRACE( "%s: image_base is not valid!", __FUNCTION__ );
	}
}

} // namespace atom

// 
// symbols
// 
namespace atom
{

void TestSymbols()
{
	SymbolData data = { };
	SymbolLoader symbol_loader = { };

	auto status = symbol_loader.Load( data );

	if( NT_SUCCESS( status ) )
	{
		TRACE( "PsLoaedModuleList @ '0x%016llX'", data.PsLoadedModuleList );
		TRACE( "PsLoadedModuleResource @ '0x%016llX'", data.PsLoadedModuleResource );
		TRACE( "PsLoadedModuleSpinLock @ '0x%016llX'", data.PsLoadedModuleSpinLock );
		TRACE_SEPARATOR();
		TRACE( "MmUnloadedDrivers @ '0x%016llX'", data.MmUnloadedDrivers );
		TRACE( "MmLastUnloadedDriver @ '0x%016llX'", data.MmLastUnloadedDriver );
		TRACE_SEPARATOR();
		TRACE( "PiDDBCacheList @ '0x%016llX'", data.PiDDBCacheList );
		TRACE( "PiDDBCacheTable @ '0x%016llX'", data.PiDDBCacheTable );
		TRACE( "PiDDBLock @ '0x%016llX'", data.PiDDBLock );
		TRACE( "PiDDBPath @ '0x%016llX'", data.PiDDBPath );
		TRACE_SEPARATOR();
		TRACE( "NtQueryVirtualMemory @ '0x%016llX'", data.NtQueryVirtualMemory );
		TRACE( "NtProtectVirtualMemory @ '0x%016llX'", data.NtProtectVirtualMemory );
		TRACE( "NtAllocateVirtualMemory @ '0x%016llX'", data.NtAllocateVirtualMemory );
		TRACE( "NtFreeVirtualMemory @ '0x%016llX'", data.NtFreeVirtualMemory );
		TRACE( "NtReadVirtualMemory @ '0x%016llX'", data.NtReadVirtualMemory );
		TRACE( "NtWriteVirtualMemory @ '0x%016llX'", data.NtWriteVirtualMemory );
		TRACE( "NtLockVirtualMemory @ '0x%016llX'", data.NtLockVirtualMemory );
		TRACE( "NtUnlockVirtualMemory @ '0x%016llX'", data.NtUnlockVirtualMemory );
	}
	else
	{
		TRACE( "%s: SymbolLoader::Load( ... ) error! (0x%08X)", __FUNCTION__, status );
	}
}

} // namespace atom

// 
// pe image
// 
namespace atom
{

void TestPEImage()
{
	namespace fs = std::filesystem;

	wchar_t windows_directory[ MAX_PATH ] = { };

	if( GetWindowsDirectoryW( windows_directory, ARRAYSIZE( windows_directory ) ) != 0 )
	{
		std::wstring ntoskrnl_path( windows_directory );
		ntoskrnl_path.append( L"\\System32\\ntoskrnl.exe" );

		win32::PEImage ntoskrnl_image = { };
		auto status = ntoskrnl_image.Load( ntoskrnl_path );

		if( NT_SUCCESS( status ) )
		{
			const auto ntoskrnl_imports = ntoskrnl_image.GetImports();

			for( const auto&[ image_name, imports ] : ntoskrnl_imports )
			{
				for( const auto& entry : imports )
				{
					if( entry.m_use_ordinal )
					{
						TRACE( "[%S] %s @ '0x%04X'", image_name.c_str(), entry.m_name.c_str(), entry.m_ordinal );
					}
					else
					{
						TRACE( "[%S] %s @ '0x%016llX'", image_name.c_str(), entry.m_name.c_str(), entry.m_address );
					}
				}
			}

			TRACE_SEPARATOR();

			for( const auto& section : ntoskrnl_image.sections() )
			{
				char name[ 16 ] = { };
				std::memcpy( name, section.Name, sizeof( section.Name ) );

				TRACE( "%s @ '0x%08X' (0x%08X)", name, section.VirtualAddress, section.Misc.VirtualSize );
			}
		}
		else
		{
			TRACE( "%s: image.Load( ... ) error! (0x%08X)", __FUNCTION__, status );
		}
	}
	else
	{
		TRACE( "%s: GetWindowsDirectoryW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
	}
}

} // namespace atom

// 
// memory
// 
namespace atom
{

void TestMemory()
{
	auto& driver = io::DriverControl::Instance();

	if( driver.Attach( nullptr ) )
	{
		auto block = io::MemoryBlock::Allocate( PAGE_SIZE );

		if( block.IsValid() )
		{
			TRACE( "block.m_address @ '0x%016llX'", block.m_address );
			TRACE( "block.m_size @ '0x%016llX'", block.m_size );

			std::uint8_t code[] =
			{
				0x48, 0xB8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov rax, 4
				0xC3,																												// retn
			};

			if( driver.WriteMemory( block.m_address, code, sizeof( code ) ) )
			{
				std::uint8_t data[ 11 ] = { };

				if( driver.ReadMemory( block.m_address, data, sizeof( data ) ) )
				{
					TRACE( "first: '0x%02X'", data[ 0 ] );
					TRACE( "last: '0x%02X'", data[ 10 ] );

					using Fn = std::int64_t( __fastcall* )();
					auto fn = Fn( block.m_address );

					auto result = fn();
					TRACE( "result: '%lld'", result );
				}
				else
				{
					TRACE( "%s: Couldn't read memory!", __FUNCTION__ );
				}
			}
			else
			{
				TRACE( "%s: Couldn't write memory!", __FUNCTION__ );
			}

			block.Free();
		}
		else
		{
			TRACE( "%s: Couldn't allocate memory page!", __FUNCTION__ );
		}

		driver.Detach();
	}
	else
	{
		TRACE( "%s: Couldn't attach to current process!", __FUNCTION__ );
	}
}

} // namespace atom

namespace atom
{

struct MapContext
{
	LARGE_INTEGER m_time_end = { };
};

std::uintptr_t GetSystemImage( const wchar_t* const name )
{
	DWORD drivers_size = 0;

	if( !EnumDeviceDrivers( nullptr, 0, &drivers_size ) )
	{
		TRACE( "%s: [1] K32EnumDeviceDrivers( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return 0;
	}

	auto drivers_count = drivers_size / sizeof( void* );
	auto drivers = std::make_unique< void* [] >( drivers_count );

	if( !EnumDeviceDrivers( drivers.get(), drivers_size, &drivers_size ) )
	{
		TRACE( "%s: [2] K32EnumDeviceDrivers( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return 0;
	}

	for( std::size_t index = 0; index < drivers_count; index++ )
	{
		const auto driver = drivers[ index ];

		if( driver )
		{
			wchar_t base_name[ MAX_PATH ] = { };

			if( GetDeviceDriverBaseNameW( driver, base_name, ARRAYSIZE( base_name ) ) != 0 )
			{
				if( constant::AreEqual( base_name, name, true ) )
				{
					return memory::ToAddress( driver );
				}
			}
		}
	}

	return 0;
}

std::uintptr_t GetSystemImage( std::uint64_t name_hash )
{
	DWORD drivers_size = 0;

	if( !EnumDeviceDrivers( nullptr, 0, &drivers_size ) )
	{
		TRACE( "%s: [1] K32EnumDeviceDrivers( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return 0;
	}

	auto drivers_count = drivers_size / sizeof( void* );
	auto drivers = std::make_unique< void* [] >( drivers_count );

	if( !EnumDeviceDrivers( drivers.get(), drivers_size, &drivers_size ) )
	{
		TRACE( "%s: [2] K32EnumDeviceDrivers( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return 0;
	}

	for( std::size_t index = 0; index < drivers_count; index++ )
	{
		const auto driver = drivers[ index ];

		if( driver )
		{
			wchar_t base_name[ MAX_PATH ] = { };

			if( GetDeviceDriverBaseNameW( driver, base_name, ARRAYSIZE( base_name ) ) != 0 )
			{
				if( constant::Hash( base_name, true ) == name_hash )
				{
					return memory::ToAddress( driver );
				}
			}
		}
	}

	return 0;
}

std::wstring GetCurrentDirectory()
{
	wchar_t current_directory[ 512 ] = { };

	if( GetCurrentDirectoryW( ARRAYSIZE( current_directory ), current_directory ) )
	{
		return { current_directory };
	}
	else
	{
		TRACE( "%s: GetCurrentDirectoryW( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
	}

	return { };
}

int QuitApplication( int quit_code )
{
	HORIZON_LOG( "%s: quit_code: %i", __FUNCTION__, quit_code );
	HORIZON_LOG_SEPARATOR();

	std::cin.get();
	return quit_code;
}

DWORD QuitProcess( int code )
{
	auto process = GetCurrentProcess();
	
	if( TerminateProcess( process, static_cast< UINT >( code ) ) == FALSE )
	{
		TRACE( "%s: TerminateProcess( ... ) error! (0x%08X)", ATOM_FUNCTION, GetLastError() );
	}

	return static_cast< DWORD >( code );
}

bool WaitForImage( const wchar_t* const name )
{
	auto& driver = io::DriverControl::Instance();

	core::Timer< core::Seconds > timer( true );
	constexpr auto timeout = std::chrono::duration_cast< std::chrono::seconds >( g_wait_image_timeout );

	while( timer.Elapsed() < timeout.count() )
	{
		const auto image = driver.GetImage( name );

		if( image != 0 )
		{
			TRACE( "Image '%S' is found @ '0x%016llX'!", name, image );
			return true;
		}

		std::this_thread::sleep_for( 10ms );
	}

	return false;
}

std::uint32_t WaitForProcess( const wchar_t* const class_name )
{
	core::Timer< core::Seconds > timer( true );
	constexpr auto timeout = std::chrono::duration_cast< std::chrono::seconds >( g_wait_process_timeout );

	while( timer.Elapsed() < timeout.count() )
	{
		const auto window = FindWindowW( class_name, nullptr );

		if( window )
		{
			DWORD process_id = 0;
			GetWindowThreadProcessId( window, &process_id );
			return static_cast< std::uint32_t >( process_id );
		}

		std::this_thread::sleep_for( 10ms );
	}

	return 0;
}

std::uint32_t GetProcessIdByName( const wchar_t* const name )
{
	const auto snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if( snapshot )
	{
		PROCESSENTRY32W entry =
		{
			sizeof( PROCESSENTRY32W )
		};

		for( auto result = Process32FirstW( snapshot, &entry );
				 result != FALSE;
				 result = Process32NextW( snapshot, &entry ) )
		{
			if( constant::AreEqual( entry.szExeFile, name, true ) )
			{
				CloseHandle( snapshot );
				return entry.th32ProcessID;
			}
		}

		CloseHandle( snapshot );
	}

	return 0;
}

std::uint32_t GetThreadIdByProcessId( std::uint32_t process_id )
{
	const auto snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, process_id );

	if( snapshot )
	{
		THREADENTRY32 entry =
		{
			sizeof( THREADENTRY32 )
		};

		std::uint32_t thread_id = 0;
		std::uint32_t thread_usage = 0;

		for( auto result = Thread32First( snapshot, &entry );
				 result != FALSE;
				 result = Thread32Next( snapshot, &entry ) )
		{
			if( entry.th32OwnerProcessID == process_id )
			{
				if( entry.cntUsage >= thread_usage )
				{
					thread_id = entry.th32ThreadID;
					thread_usage = entry.cntUsage;
					break;
				}
			}
		}

		CloseHandle( snapshot );
		return thread_id;
	}

	return 0;
}

bool IsDriverLoaded()
{
	auto& driver = io::DriverControl::Instance();

	// 
	// if no driver open
	// 
	if( !driver.IsValid() )
	{
		// 
		// try to open one
		// 
		if( !driver.Create() )
		{
			TRACE( "%s: driver.Create( ... ) error!", __FUNCTION__ );
			return false;
		}
	}

	std::uint32_t major = 0;
	std::uint32_t minor = 0;

	// 
	// try to query version
	// 
	if( !driver.QueryDriverVersion( &major, &minor ) )
	{
		TRACE( "%s: driver.QueryVersion( ... ) error!", __FUNCTION__ );
		return false;
	}

	// 
	// return success if version matches
	// 
	return ( major == 1 &&
					 minor == 5 );
}

HWND g_window = nullptr;
std::uint32_t g_thread_id = 0;

BOOL API_STDCALL EnumerateWindow( HWND window, LPARAM lparam )
{
	DWORD process_id = 0;
	const auto thread_id = GetWindowThreadProcessId( window, &process_id );

	if( process_id == static_cast< DWORD >( lparam ) )
	{
		g_window = window;
		g_thread_id = thread_id;
		return FALSE;
	}

	return TRUE;
}

void TestMMap()
{
	auto& doze = io::DriverControl::Instance();

	if( !doze.IsValid() )
	{
		TRACE( "%s: Doze is not valid!", __FUNCTION__ );
		return;
	}

	doze.Attach( nullptr );
	// const auto process_id = GetProcessIdByName( L"notepad.exe" );
	// TRACE( "process_id = '0x%08X'", process_id );

	// if( process_id )
	{
		// EnumWindows( &EnumerateWindow, LPARAM( process_id ) );

		// if( g_thread_id )
		{
			// doze.SetWindow( g_window );
			// doze.SetThreadId( g_thread_id );
			// doze.SetProcessId( process_id );

			const auto name = "GetProcAddress"s;

			const auto image_kernel32 = GetModuleHandleW( L"kernel32.dll" );
			const auto image_export = std::uintptr_t( GetProcAddress( image_kernel32, name.c_str() ) );

			const auto remote_name = doze.Commit( PAGE_SIZE );
			doze.WriteMemory( remote_name.m_address, name.c_str(), name.size() );

			const auto remote_export = ExecuteProcedure( doze.GetProcessId(), doze.GetThreadId(), image_export, std::uint64_t( image_kernel32 ), remote_name.m_address, 0 );

			TRACE( "image_export = '0x%016llX'", image_export );
			TRACE( "remote_export = '0x%016llX'", remote_export );
		}
	}
}	

bool DriverMapUserImage()
{
	auto& driver = io::DriverControl::Instance();

	if( !driver.IsValid() )
	{
		TRACE( "%s: Driver is not valid!", __FUNCTION__ );
		return false;
	}

	if( !driver.QueryDriverSecure( cx_intel_driver_name, cx_intel_time_date_stamp ) )
	{
		TRACE( "%s: ExecuteSecurity( '%S', '0x%08X' ) error!", __FUNCTION__, cx_intel_driver_name, cx_intel_time_date_stamp );
	}

	// 
	// Dump BattlEye shellcode
	// 
	// return DumpBattlEye();

	const auto process_id = WaitForProcess( cx_game_class_name );

	if( !process_id )
	{
		TRACE( "%s: Couldn't acquire process!", __FUNCTION__ );
		return false;
	}

	if( !driver.Attach( cx_game_class_name ) )
	{
		TRACE( "%s: Couldn't attach to process!", __FUNCTION__ );
		return false;
	}

	if( !WaitForImage( cx_game_image_name ) )
	{
		TRACE( "%s: Couldn't acquire '%S' image!", __FUNCTION__, cx_game_image_name );
		return false;
	}

	std::size_t image_size = 0;
	std::unique_ptr< std::uint8_t[] > image_data = { };

	if( g_use_debug_image )
	{
		std::wstring directory_current( GetCurrentDirectory() );
		std::wstring directory_image( directory_current + L"\\" + cx_game_file_name );

		std::vector< std::uint8_t > image = { };

		if( win32::SafeReadFile( directory_image, image ) )
		{
			image_size = image.size();
			image_data = std::make_unique< std::uint8_t[] >( image_size );

			if( image_data )
			{
				std::memcpy( image_data.get(), image.data(), image.size() );
			}
		}
	}
	else
	{
		image_size = resource::m_pubg_size;
		image_data = std::make_unique< std::uint8_t[] >( image_size );

		for( std::size_t index = 0; index < image_size; index++ )
		{
			image_data[ index ] = resource::m_pubg_data[ index ] ^ resource::m_pubg_key;
		}
	}

	// 
	// map user image to process
	// 
	if( !MapUserImage( image_data.get(), image_size ) )
	{
		TRACE( "%s: Couldn't map use image!", __FUNCTION__ );
		return false;
	}

	return true;
}

#if defined( HORIZON_DEBUG )
#define HORIZON_KERNEL_NAME		L"doze.sys"
#elif defined( HORIZON_RELEASE )
#define HORIZON_KERNEL_NAME		L"doze.sys"
#endif // HORIZON_DEBUG

bool MapImageKernel( kernel::Loader& loader, resource::DataLoader& data_loader )
{
	HORIZON_LOG_SEPARATOR();

	std::size_t image_size = 0;
	std::unique_ptr< std::uint8_t[] > image_data = { };

	if( g_use_debug_doze )
	{
		std::wstring directory_current( GetCurrentDirectory() );
		std::wstring directory_image( directory_current + L"\\doze.sys" );

		std::vector< std::uint8_t > image = { };

		if( win32::SafeReadFile( directory_image, image ) )
		{
			image_size = image.size();
			image_data = std::make_unique< std::uint8_t[] >( image_size );

			if( image_data )
			{
				std::memcpy( image_data.get(), image.data(), image.size() );
			}
		}
	}
	else
	{
		image_size = horizon::resource::m_doze_size;
		image_data = std::make_unique< std::uint8_t[] >( image_size );

		for( std::size_t index = 0; index < image_size; index++ )
		{
			image_data[ index ] = horizon::resource::m_doze_data[ index ] ^ horizon::resource::m_doze_key;
		}
	}

	if( !loader.Map( image_data.get(), image_size ) )
	{
		HORIZON_LOG( "Can't map \"%s\" driver!", "herby" );
		return false;
	}

	return true;
}

void DeleteDriver( const std::wstring& path )
{
	try
	{
		if( !fs::remove( path ) )
		{
			TRACE( "%s: fs::remove( ... ) error!", __FUNCTION__ );
		}
		else
		{
			TRACE( "Driver was deleted @ '%S'!", path.c_str() );
		}
	}
	catch( const std::exception & e )
	{
		TRACE( "%s: fs::remove( ... ) exception! (%s)", __FUNCTION__, e.what() );
	}
}

void DriverUnload()
{
	// auto& doze = io::DriverControl::Instance();

	// if( !doze.QueryDriverQuit() )
	{
		// TRACE( "%s: QueryDriverQuit() error!", __FUNCTION__ );
	}
}

int OnImageDispatch()
{
	if( !GrantRequiredPrivileges() )
	{
		TRACE( "%s: GrantRequiredPrivileges() error!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	if( !g_use_debug_doze_load )
	{
		// 
		// look if driver was already loaded
		// 
		if( IsDriverLoaded() )
		{
			// 
			// try to map user image to process
			// 
			if( !DriverMapUserImage() )
			{
				TRACE( "%s: Couldn't map user image to process!", __FUNCTION__ );
				return EXIT_FAILURE;
			}

			DriverUnload();
			return EXIT_SUCCESS;
		}
	}

	resource::DataLoader data_loader( cx_intel_driver_name );

	if( !data_loader.IsValid() )
	{
		TRACE( "%s: data_loader.IsValid() error!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	auto driver_path = data_loader.GetDriverPath();
	kernel::Loader loader( driver_path, cx_intel_device_name );
	
	if( !loader.Load() )
	{
		TRACE( "%s: loader.Load() error!", __FUNCTION__ );
		return EXIT_FAILURE;
	}
	
	if( !loader.InitializeUserCode() || !loader.InitializeKernelCode() )
	{
		TRACE( "%s: Can't initialize loader!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	if( !MapImageKernel( loader, data_loader ) )
	{
		TRACE( "%s: Can't map \"Intel\" driver!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	if( !loader.Unload() )
	{
		TRACE( "%s: Can't unload \"Intel\" driver!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	DeleteDriver( driver_path );

	if( !IsDriverLoaded() )
	{
		TRACE( "%s: Driver is not loaded!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	if( !DriverMapUserImage() )
	{
		TRACE( "%s: Couldn't map user image to process!", __FUNCTION__ );
		return EXIT_FAILURE;
	}

	DriverUnload();
	return EXIT_SUCCESS;
}

bool EnsureDependencyLoaded( const std::wstring& name )
{
	if( name.empty() )
	{
		return true;
	}

	auto image = GetModuleHandleW( name.c_str() );

	if( image  )
	{
		return true;
	}

	image = LoadLibraryW( name.c_str() );

	if( image )
	{
		return true;
	}

	return false;
}

bool EnsureDependencies()
{
	return ( EnsureDependencyLoaded( L"ole32.dll" ) &&
					 EnsureDependencyLoaded( L"msdia140.dll" ) );
}

DWORD API_WIN32 ImageExecute( void* parameter )
{
	auto code = EXIT_FAILURE;

	if( EnsureDependencies() )
	{
		code = OnImageDispatch();
	}
	else
	{
		TRACE( "%s: EnsureDependencies() error!", ATOM_FUNCTION );
	}

	/*if( parameter )
	{
		LARGE_INTEGER counter = { };
		QueryPerformanceCounter( &counter );
		g_time_expire.QuadPart = reinterpret_cast< LONGLONG >( parameter );

		if( counter.QuadPart < g_time_expire.QuadPart )
		{
			if( EnsureDependencies() )
			{
				code = OnImageDispatch();
			}
			else
			{
				TRACE( "%s: EnsureDependencies() error!", ATOM_FUNCTION );
			}
		}
		else
		{
			TRACE( "%s: Subscription has expired!", ATOM_FUNCTION );
		}
	}
	else
	{
		TRACE( "%s: parameter is nullptr!", ATOM_FUNCTION );
	}*/
	
	return QuitProcess( code );
}

void OnImageLoad( void* instance, void* reserved )
{
	// 
	// prevent DLL_THREAD_ATTACH / DLL_THREAD_DETACH reason calls
	// 
	DisableThreadLibraryCalls( static_cast< HMODULE >( instance ) );

	// 
	// spawn main thread
	// 
	auto execute = CreateThread( nullptr, 0, &ImageExecute, reserved, 0, nullptr );

	if( win32::IsHandleValid( execute ) )
	{
		// 
		// don't leak handles
		// 
		CloseHandle( execute );
	}
}

void OnImageFree( void* reserved )
{ }

int ImageDispatch( void* instance, std::uint32_t reason, void* reserved )
{
	switch( reason )
	{
		case DLL_PROCESS_ATTACH:
		{
			OnImageLoad( instance, reserved );
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			OnImageFree( reserved );
			break;
		}
	}

	return TRUE;
}

} // namespace atom

HORIZON_WARNING_PUSH( 28251 )

int API_STDCALL wWinMain( HINSTANCE, HINSTANCE, wchar_t*, int )
{
	if( g_use_debug_console )
	{
		// 
		// create console window & attach it to current process
		// 
		AllocConsole();
		AttachConsole( GetCurrentProcessId() );

		FILE* con_stdin = nullptr;
		FILE* con_stdout = nullptr;
		FILE* con_stderr = nullptr;

		// 
		// open console input & output
		// 
		freopen_s( &con_stdin, "CONIN$", "r", stdin );
		freopen_s( &con_stdout, "CONOUT$", "w", stdout );
		freopen_s( &con_stderr, "CONOUT$", "w", stderr );
		
		// 
		// change console window caption
		// 
		SetConsoleTitleW( L"[krypton.loader] debug console" );
	}

	const auto result = atom::OnImageDispatch();

	if( g_use_debug_console )
	{
		// 
		// destroy console window
		// 
		FreeConsole();
	}

	return result;
}

HORIZON_WARNING_POP()

int API_STDCALL DllMain( void* instance, unsigned long reason, void* reserved )
{
	return atom::ImageDispatch( instance, reason, reserved );
}