#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/portable_executable.hpp"

#include "../win32/trace.hpp"
#include "../win32/process.hpp"

namespace atom::memory
{

constexpr bool is_terminator( char character )
{
	return ( character == static_cast< char >( 0 ) );
}

constexpr bool is_space( char character )
{
	return ( character == ' ' );
}

constexpr bool is_question( char character )
{
	return ( character == '?' );
}

constexpr bool is_decimal( char character )
{
	return ( character >= '0' && character <= '9' );
}

constexpr bool is_hexadecimal_lower( char character )
{
	return ( character >= 'a' && character <= 'f' );
}

constexpr bool is_hexadecimal_upper( char character )
{
	return ( character >= 'A' && character <= 'F' );
}

constexpr int to_bits( char character )
{
	if( is_decimal( character ) )
		return static_cast< int >( character - '0' );
	else if( is_hexadecimal_lower( character ) )
		return static_cast< int >( character - 'a' + 0x0A );
	else if( is_hexadecimal_upper( character ) )
		return static_cast< int >( character - 'A' + 0x0A );
	else
		return static_cast< int >( -1 );
}

constexpr int to_bits_pair( int high, int low )
{
	return ( ( high & 0x0F ) << 4 | ( low & 0x0F ) );
}

template< typename T >
constexpr int to_byte( const T* data )
{
	return to_bits_pair( to_bits( data[ 0 ] ), to_bits( data[ 1 ] ) );
}

std::uintptr_t ScanRegionInternal( const std::uint8_t* region_begin, const std::uint8_t* region_end, const char* signature );

std::uintptr_t ScanImageSectionInternal( std::uintptr_t image, const char* name, const char* signature );
std::uintptr_t ScanImageInternal( std::uintptr_t image, const char* signature );

std::uintptr_t ScanSectionInternal( const char* name, const char* signature );
std::uintptr_t ScanInternal( const char* signature );

std::uintptr_t RipInternal( std::uintptr_t address, std::size_t byte_count );

template< typename T = std::uintptr_t >
T ScanRegion( const std::uint8_t* region_begin, const std::uint8_t* region_end, const char* signature )
{
	return T( ScanRegionInternal( region_begin, region_end, signature ) );
}

template< typename T = std::uintptr_t >
T ScanImageSection( std::uintptr_t image, const char* name, const char* signature )
{
	return T( ScanImageSectionInternal( image, name, signature ) );
}

template< typename T = std::uintptr_t >
T ScanImage( std::uintptr_t image, const char* signature )
{
	return T( ScanImageInternal( image, signature ) );
}

template< typename T = std::uintptr_t >
T ScanSection( const char* name, const char* signature )
{
	return T( ScanSectionInternal( name, signature ) );
}

template< typename T = std::uintptr_t >
T Scan( const char* signature )
{
	return T( ScanInternal( signature ) );
}

template< typename T = std::uintptr_t >
T GetAbsoluteAddress( std::uintptr_t address, std::size_t byte_count )
{
	return T( RipInternal( address, byte_count ) );
}

}