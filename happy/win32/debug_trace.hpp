#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::win32
{

void DebugMessage( const char* message );
void DebugMessage( const wchar_t* message );

void TraceFormat( const char* format, va_list args );
void TraceFormat( const wchar_t* format, va_list args );

template< typename T >
void Trace( const T* format, ... )
{
	va_list args = { };
	va_start( args, format );
	TraceFormat( format, args );
	va_end( args );
}

} // namespace atom::win32

#if defined( HORIZON_DEBUG )
	#define HORIZON_TRACE( format, ... )	atom::win32::Trace( ##format, ##__VA_ARGS__ )
#elif defined( HORIZON_RELEASE )
	// #define HORIZON_TRACE( format, ... )	atom::win32::Trace( ##format, ##__VA_ARGS__ )
	#define HORIZON_TRACE( ... )					( __VA_ARGS__ )
#endif // HORIZON_DEBUG