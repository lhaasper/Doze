#include "symbol_loader.hpp"
#include "pdb_manager.hpp"

#include "../win32/console.hpp"

namespace atom
{

SymbolLoader::SymbolLoader()
	: m_x86_os( false )
	, m_wow64_process( false )
{
	SYSTEM_INFO info = { };
	GetNativeSystemInfo( &info );

	if( info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
	{
		m_x86_os = true;
	}
	else
	{
		BOOL wow64 = FALSE;
		IsWow64Process( GetCurrentProcess(), &wow64 );
		m_wow64_process = ( wow64 != FALSE );
	}
}

NTSTATUS SymbolLoader::Load( SymbolData& symbols )
{
	return LoadFromSymbols( symbols );
}

NTSTATUS SymbolLoader::LoadFromSymbols( SymbolData& symbols )
{
	PDBManager sym64 = { };

	wchar_t windows_directory[ MAX_PATH ] = { };

	if( GetWindowsDirectoryW( windows_directory, ARRAYSIZE( windows_directory ) ) == 0 )
	{
		TRACE( "%s: GetWindowsDirectoryW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return NTSTATUS_FROM_WIN32( GetLastError() );
	}

	std::wstring system32_directory( windows_directory + L"\\System32\\"s );
	std::wstring ntoskrnl_path( system32_directory + L"ntoskrnl.exe"s );
	
	TRACE( "ntoskrnl_path = '%ws'", ntoskrnl_path.c_str() );

	auto ntoskrnl = LoadLibraryExW( ntoskrnl_path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES );
	TRACE( "ntoskrnl = '0x%016llX'", std::uintptr_t( ntoskrnl ) );

	if( !ntoskrnl )
	{
		TRACE( "%s: LoadLibraryExW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return NTSTATUS_FROM_WIN32( GetLastError() );
	}

	TRACE( "sym64.Initialize( ... ) begin" );
	auto result = sym64.Initialize( ntoskrnl_path.c_str(), std::uintptr_t( ntoskrnl ) );
	TRACE( "sym64.Initialize( ... ) end = '0x%08X'", result );
	
	if( FAILED( result ) )
	{
		TRACE( "%s: sym64.Initialize( ... ) error! (0x%08X)", __FUNCTION__, result );
		return static_cast< NTSTATUS >( result );
	}
	
	sym64.GetSymbol( L"PsLoadedModuleList", symbols.PsLoadedModuleList );
	sym64.GetSymbol( L"PsLoadedModuleResource", symbols.PsLoadedModuleResource );
	sym64.GetSymbol( L"PsLoadedModuleSpinLock", symbols.PsLoadedModuleSpinLock );
	sym64.GetSymbol( L"PsInvertedFunctionTable", symbols.PsInvertedFunctionTable );

	sym64.GetSymbol( L"MmUnloadedDrivers", symbols.MmUnloadedDrivers );
	sym64.GetSymbol( L"MmLastUnloadedDriver", symbols.MmLastUnloadedDriver );

	sym64.GetSymbol( L"PiDDBCacheList", symbols.PiDDBCacheList );
	sym64.GetSymbol( L"PiDDBCacheTable", symbols.PiDDBCacheTable );
	sym64.GetSymbol( L"PiDDBLock", symbols.PiDDBLock );
	sym64.GetSymbol( L"PiDDBPath", symbols.PiDDBPath );

	sym64.GetSymbol( L"RtlInsertInvertedFunctionTable", symbols.RtlInsertInvertedFunctionTable );

	sym64.GetSymbol( L"NtQueryVirtualMemory", symbols.NtQueryVirtualMemory );
	sym64.GetSymbol( L"NtProtectVirtualMemory", symbols.NtProtectVirtualMemory );
	sym64.GetSymbol( L"NtAllocateVirtualMemory", symbols.NtAllocateVirtualMemory );
	sym64.GetSymbol( L"NtFreeVirtualMemory", symbols.NtFreeVirtualMemory );
	sym64.GetSymbol( L"NtReadVirtualMemory", symbols.NtReadVirtualMemory );
	sym64.GetSymbol( L"NtWriteVirtualMemory", symbols.NtWriteVirtualMemory );
	sym64.GetSymbol( L"NtLockVirtualMemory", symbols.NtLockVirtualMemory );
	sym64.GetSymbol( L"NtUnlockVirtualMemory", symbols.NtUnlockVirtualMemory );

	FreeLibrary( ntoskrnl );
	return static_cast< NTSTATUS >( result );
}

} // namespace atom