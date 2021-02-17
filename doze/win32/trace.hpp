#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::win32
{

void DebugMessage( const char* const message );
void DebugMessage( const wchar_t* const message );

void TraceFormat( const char* const format, va_list arg_pack );
void TraceFormat( const wchar_t* const format, va_list arg_pack );

template< typename Type >
void Trace( const Type* const format, ... )
{
	va_list arg_pack = { };
	va_start( arg_pack, format );

	TraceFormat( format, arg_pack );
	
	va_end( arg_pack );
}

} // namespace atom::win32

#if defined( ATOM_DEBUG )
#define TRACE( Format, ... )				atom::win32::Trace( Format, __VA_ARGS__ )
#elif defined( ATOM_RELEASE )
#define TRACE( Format, ... )				atom::win32::Trace( Format, __VA_ARGS__ )
// #define TRACE( ... )								( __VA_ARGS__ )
#endif // ATOM_DEBUG

#define TRACE_SEPARATOR()						TRACE( "============================================================" )