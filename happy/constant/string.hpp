#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

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

template< typename Type >
constexpr bool AreEqual( const Type* const source, const Type* const destination, std::size_t length, bool ignore_case )
{
	for( std::size_t index = 0; index < length; index++ )
	{
		if( ignore_case )
		{
			if( ToLower( source[ index ] ) != ToLower( destination[ index ] ) )
			{
				// 
				// break and return failure
				// 
				return false;
			}
		}
		else
		{
			if( source[ index ] != destination[ index ] )
			{
				// 
				// break and return failure
				// 
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
		// 
		// length mismatch
		// 
		return false;
	}

	return AreEqual( source, destination, source_length, ignore_case );
}

} // namespace atom::constant