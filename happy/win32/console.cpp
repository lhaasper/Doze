#include "console.hpp"
#include "debug_trace.hpp"

namespace atom::win32
{

Console::~Console()
{
	Destroy();
}

bool Console::Create( const std::wstring& caption )
{
	Destroy();

	m_process_id = static_cast< std::uint32_t >( GetCurrentProcessId() );

	if( !m_process_id )
	{
		HORIZON_TRACE( "%s: GetCurrentProcessId() error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	m_caption = caption;

	if( m_caption.empty() )
	{
		HORIZON_TRACE( "%s: m_caption is empty!", __FUNCTION__ );
		return false;
	}

	if( !AllocConsole() )
	{
		HORIZON_TRACE( "%s: AllocConsole() error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !AttachConsole( static_cast< DWORD >( m_process_id ) ) && GetLastError() != ERROR_ACCESS_DENIED )
	{
		HORIZON_TRACE( "%s: AttachConsole( 0x%08X ) error! (0x%08X)", __FUNCTION__, m_process_id, GetLastError() );
		return false;
	}

	if( !SetConsoleTitleW( m_caption.c_str() ) )
	{
		HORIZON_TRACE( "%s: SetConsoleTitleW( \"%ws\" ) error! (0x%08X)", __FUNCTION__, m_caption.c_str(), GetLastError() );
		return false;
	}

	m_std_input_handle = GetStdHandle( STD_INPUT_HANDLE );
	m_std_output_handle = GetStdHandle( STD_OUTPUT_HANDLE );
	m_std_error_handle = GetStdHandle( STD_ERROR_HANDLE );
	
	auto code = freopen_s( &m_std_input, "CONIN$", "r", stdin );
	
	if( code != 0 )
	{
		TRACE( "%s: freopen_s( ... ) error! (0x%08X)", ATOM_FUNCTION, code );
		return false;
	}

	code = freopen_s( &m_std_output, "CONOUT$", "w", stdout );

	if( code != 0 )
	{
		TRACE( "%s: freopen_s( ... ) error! (0x%08X)", ATOM_FUNCTION, code );
		return false;
	}

	code = freopen_s( &m_std_error, "CONOUT$", "w", stderr );

	if( code != 0 )
	{
		TRACE( "%s: freopen_s( ... ) error! (0x%08X)", ATOM_FUNCTION, code );
		return false;
	}

	return true;
}

void Console::Destroy()
{
	if( m_std_input )
	{
		fclose( m_std_input );
	}

	if( m_std_output )
	{
		fclose( m_std_output );
	}

	if( m_std_error )
	{
		fclose( m_std_error );
	}

	if( m_process_id || !m_caption.empty() )
	{
		FreeConsole();
	}

	m_process_id = 0;
	m_caption.clear();

	// m_conout_stdout = nullptr;
	// m_conout_stderr = nullptr;
}

void Console::LogV( const char* format, va_list args )
{
	char message[ 2048 ] = { };
	vsprintf_s( message, format, args );

	char output[ 4096 ] = { };
	const auto length = sprintf_s( output, "[atom] %s\r\n", message );

	std::printf( output );
	DebugMessage( output );
}

void Console::LogV( const wchar_t* format, va_list args )
{
	wchar_t message[ 2048 ] = { };
	vswprintf_s( message, format, args );

	wchar_t output[ 4096 ] = { };
	const auto length = swprintf_s( output, L"[atom] %s\r\n", message );

	std::wprintf( output );
	DebugMessage( output );
}

} // namespace atom::win32