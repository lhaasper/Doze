#include "pdb_manager.hpp"

#include "../win32/console.hpp"
#include "../win32/import_manager.hpp"

namespace fs = std::filesystem;

namespace atom
{

PDBManager::PDBManager()
{
	auto result = IMPORT_CALL( CoInitialize, nullptr );
	
	if( FAILED( result ) )
	{
		TRACE( "%s: CoInitialize() error! (0x%08X)", __FUNCTION__, result );
		return;
	}

	wchar_t path[ MAX_PATH ] = { };

	if( GetTempPathW( ARRAYSIZE( path ), path ) != 0 )
	{
		try
		{
			fs::remove( path + L"Symbols"s );
		}
		catch( std::exception& e )
		{ }
		
		std::wstringstream stream;
		stream << L"srv*" << path << L"Symbols*http://msdl.microsoft.com/download/symbols";
		m_directory = stream.str();
	}
	else
	{
		TRACE( "%s: GetTempPathW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
	}
}

PDBManager::~PDBManager()
{
	m_scope.Release();
	m_session.Release();
	m_source.Release();

	IMPORT_CALL( CoUninitialize );
}

HRESULT PDBManager::Initialize( const std::wstring& file, std::uintptr_t base /*= 0*/ )
{
	auto result = CreateDiaDataSource();
	
	if( FAILED( result ) )
	{
		TRACE( "%s: CreateDiaDataSource() error! (0x%08X)", __FUNCTION__, result );
		return result;
	}

	result = m_source->loadDataForExe( file.c_str(), m_directory.empty() ? nullptr : m_directory.c_str(), nullptr );

	if( FAILED( result ) )
	{
		TRACE( "%s: m_source->loadDataForExe( ... ) error! (0x%08X)", __FUNCTION__, result );
		return result;
	}

	result = m_source->openSession( &m_session );
	
	if( FAILED( result ) )
	{
		TRACE( "%s: m_source->openSession( ... ) error! (0x%08X)", __FUNCTION__, result );
		return result;
	}

	result = m_session->get_globalScope( &m_scope );

	if( FAILED( result ) )
	{
		TRACE( "%s: m_session->get_globalScope( ... ) error! (0x%08X)", __FUNCTION__, result );
		return result;
	}

	m_base = base;
	return PopulateSymbols();
}

HRESULT PDBManager::GetSymbol( const std::wstring& name, std::uintptr_t& result )
{
	auto symbol = m_symbols.find( name );

	if( symbol != m_symbols.end() )
	{
		result = symbol->second;
		return S_OK;
	}

	return E_PDB_DBG_NOT_FOUND;
}

HRESULT PDBManager::CreateDiaDataSource()
{
	auto result = IMPORT_CALL( CoCreateInstance, CLSID_DiaSource, nullptr, CLSCTX_INPROC_SERVER, IID_IDiaDataSource, ( PVOID* )&m_source );
	
	if( result == REGDB_E_CLASSNOTREG )
	{
		CComPtr< IClassFactory > class_factory = { };
		result = IMPORT_CALL( DllGetClassObject, CLSID_DiaSource, IID_IClassFactory, &class_factory );

		if( FAILED( result ) )
		{
			TRACE( "%s: DllGetClassObject( ... ) error! (0x%08X)", __FUNCTION__, result );
			return result;
		}

		result = class_factory->CreateInstance( nullptr, IID_IDiaDataSource, ( PVOID* )&m_source );

		if( FAILED( result ) )
		{
			TRACE( "%s: class_factory->CreateInstance( ... ) error! (0x%08X)", __FUNCTION__, result );
		}
	}
	else if( FAILED( result ) )
	{
		TRACE( "%s: CoCreateInstance( ... ) error! (0x%08X)", __FUNCTION__, result );
	}
	
	return result;
}

HRESULT PDBManager::PopulateSymbols()
{
	CComPtr< IDiaEnumSymbols > symbols = { };

	auto result = m_scope->findChildren( SymTagNull, nullptr, nsNone, &symbols );

	if( FAILED( result ) )
	{
		TRACE( "%s: m_scope->findChildren( ... ) error! (0x%08X)", __FUNCTION__, result );
		return result;
	}

	ULONG count = 0;
	CComPtr< IDiaSymbol > symbol = { };
	
	while( SUCCEEDED( symbols->Next( 1, &symbol, &count ) ) && count != 0 )
	{
		DWORD rva = 0;
		BSTR name = nullptr;

		symbol->get_relativeVirtualAddress( &rva );
		symbol->get_undecoratedName( &name );

		if( rva && name )
		{
			std::wstring wide_name( name );

			if( wide_name[ 0 ] == L'@' || wide_name[ 0 ] == L'_' )
			{
				wide_name.erase( 0, 1 );
			}

			auto location = wide_name.rfind( L'@' );

			if( location != wide_name.npos )
			{
				wide_name.erase( location );
			}

			m_symbols.emplace( std::move( wide_name ), rva );
		}

		symbol.Release();
	}

	return S_OK;
}

} // namespace atom