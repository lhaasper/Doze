#include "trace.hpp"

namespace atom::win32
{

void DebugMessage( const char* const message )
{
	DbgPrint( "[atom::doze] %s\n", message );
}

void DebugMessage( const wchar_t* const message )
{
	DbgPrint( "[atom::doze] %S\n", message );
}

void TraceFormat( const char* const format, va_list arg_pack )
{
	const auto length = vsprintf_s( nullptr, 0, format, arg_pack );

	if( length > 0 )
	{
		const auto message = new char[ length ];

		if( message )
		{
			vsprintf_s( message, length, format, arg_pack );

			DebugMessage( message );

			delete[] message;
		}
	}
	else
	{
		char message[ 4096 ] = { };
		vsprintf_s( message, ARRAYSIZE( message ), format, arg_pack );

		DebugMessage( message );
	}
}

void TraceFormat( const wchar_t* const format, va_list arg_pack )
{
	const auto length = vswprintf_s( nullptr, 0, format, arg_pack );

	if( length > 0 )
	{
		const auto message = new wchar_t[ length ];

		if( message )
		{
			vswprintf_s( message, length, format, arg_pack );

			DebugMessage( message );

			delete[] message;
		}
	}
	else
	{
		wchar_t message[ 4096 ] = { };
		vswprintf_s( message, ARRAYSIZE( message ), format, arg_pack );

		DebugMessage( message );
	}
}

} // namespace atom::win32