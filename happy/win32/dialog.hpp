#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::win32
{

enum EDialogType : std::uint32_t
{
	DialogMessage,
	DialogWarning,
	DialogError,
};

int DialogExecute( const char* caption, const char* message, std::uint32_t button );
int DialogExecute( const wchar_t* caption, const wchar_t* message, std::uint32_t button );

int DialogV( std::uint32_t dialog_type, const char* format, va_list args );
int DialogV( std::uint32_t dialog_type, const wchar_t* format, va_list args );

template< typename T >
int Message( const T* format, ... )
{
	va_list args = { };
	va_start( args, format );
	const auto code = DialogV( DialogMessage, format, args );
	va_end( args );
	return code;
}

template< typename T >
int Warning( const T* format, ... )
{
	va_list args = { };
	va_start( args, format );
	const auto code = DialogV( DialogWarning, format, args );
	va_end( args );
	return code;
}

template< typename T >
int Error( const T* format, ... )
{
	va_list args = { };
	va_start( args, format );
	const auto code = DialogV( DialogError, format, args );
	va_end( args );
	return code;
}

}