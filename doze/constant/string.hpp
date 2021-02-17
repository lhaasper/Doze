#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/string.hpp"
#include "../win32/sdk/unicode_string.hpp"

#include "character.hpp"

namespace atom::constant
{

template< typename Type >
constexpr std::size_t GetLength( const Type* const data )
{
	std::size_t length = 0;

	while( true )
	{
		if( IsTerminator( data[ length ] ) )
		{
			break;
		}

		length++;
	}

	return length;
}

constexpr std::size_t GetLength( const win32::STRING& data )
{
	const auto length = data.Length / sizeof( char );
	return length;
}

constexpr std::size_t GetLength( const win32::UNICODE_STRING& data )
{
	const auto length = data.Length / sizeof( wchar_t );
	return length;
}

template< typename Type >
constexpr bool AreEqual( const Type* const source, const Type* const destination, std::size_t length, bool ignore_case )
{
	for( std::size_t index = 0; index < length; index++ )
	{
		if( ignore_case )
		{
			if( ToLower( source[ index ] ) != ToLower( destination[ index ] ) )
			{
				return false;
			}
		}
		else
		{
			if( source[ index ] != destination[ index ] )
			{
				return false;
			}
		}
	}

	return true;
}

template< typename Type >
constexpr bool AreEqual( const Type* const source, const Type* const destination, bool ignore_case )
{
	const auto source_length = GetLength( source );
	const auto destination_length = GetLength( destination );

	if( source_length != destination_length )
	{
		return false;
	}

	return AreEqual( source, destination, source_length, ignore_case );
}

constexpr bool AreEqual( const win32::STRING& source, const char* const destination, bool ignore_case )
{
	const auto source_length = GetLength( source );
	const auto destination_length = GetLength( destination );

	if( source_length != destination_length )
	{
		return false;
	}

	return AreEqual( source.Buffer, destination, source_length, ignore_case );
}

constexpr bool AreEqual( const win32::UNICODE_STRING& source, const wchar_t* const destination, bool ignore_case )
{
	const auto source_length = GetLength( source );
	const auto destination_length = GetLength( destination );

	if( source_length != destination_length )
	{
		return false;
	}

	return AreEqual( source.Buffer, destination, source_length, ignore_case );
}

} // namespace atom::constant