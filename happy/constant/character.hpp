#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::constant
{

template< typename Type >
constexpr bool IsTerminator( const Type character )
{
	return ( character == static_cast< const Type >( '\0' ) );
}

template< typename Type >
constexpr bool IsQuestion( const Type character )
{
	return ( character == static_cast< const Type >( '?' ) );
}

template< typename Type >
constexpr bool IsLower( const Type character )
{
	return ( character >= static_cast< const Type >( 'a' ) &&
					 character <= static_cast< const Type >( 'z' ) );
}

template< typename Type >
constexpr bool IsUpper( const Type character )
{
	return ( character >= static_cast< const Type >( 'A' ) &&
					 character <= static_cast< const Type >( 'Z' ) );
}

template< typename Type >
constexpr Type ToLower( const Type character )
{
	if( IsUpper( character ) )
	{
		return ( character + static_cast< const Type >( 32 ) );
	}

	return character;
}

template< typename Type >
constexpr Type ToUpper( const Type character )
{
	if( IsLower( character ) )
	{
		return ( character - static_cast< const Type >( 32 ) );
	}

	return character;
}

} // namespace atom::constant