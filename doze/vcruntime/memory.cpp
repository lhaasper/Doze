#include "memory.hpp"

namespace atom::vcruntime
{

#pragma function( memcpy )
void* memcpy( void* const destination, const void* const source, std::size_t size )
{
	const auto data_source = reinterpret_cast< const std::uint8_t* >( source );
	const auto data_destination = reinterpret_cast< std::uint8_t* >( destination );

	__movsb( data_destination, data_source, size );
	
	return destination;
}

#pragma function( memmove )
void* memmove( void* const destination, const void* const source, std::size_t size )
{
	const auto data_source = reinterpret_cast< const std::uint8_t* >( source );
	const auto data_destination = reinterpret_cast< std::uint8_t* >( destination );

	__movsb( data_destination, data_source, size );

	return destination;
}

#pragma function( memset )
void* memset( void* const destination, int value, std::size_t size )
{
	const auto data_value = static_cast< std::uint8_t >( value );
	const auto data_destination = reinterpret_cast< std::uint8_t* >( destination );

	__stosb( data_destination, data_value, size );

	return destination;
}

} // namespace atom::vcruntime