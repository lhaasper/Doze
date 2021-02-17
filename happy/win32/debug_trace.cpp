#include "debug_trace.hpp"

namespace atom::win32
{

void DebugMessage( const char* message )
{
	OutputDebugStringA( message );
}

void DebugMessage( const wchar_t* message )
{
	OutputDebugStringW( message );
}

void TraceFormat( const char* format, va_list args )
{
	char message[ 2048 ] = { };
	vsprintf_s( message, format, args );

	char output[ 4096 ] = { };
	sprintf_s( output, "[atom] %s\r\n", message );

	DebugMessage( output );
}

void TraceFormat( const wchar_t* format, va_list args )
{
	wchar_t message[ 2048 ] = { };
	vswprintf_s( message, format, args );

	wchar_t output[ 4096 ] = { };
	swprintf_s( output, L"[atom] %s\r\n", message );

	DebugMessage( output );
}

}