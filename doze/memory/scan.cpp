#include "scan.hpp"
#include "operation.hpp"

#include "../constant/hash.hpp"
#include "../constant/string.hpp"

// #include "../win32/sdk/portable_executable.hpp"

#include "../win32/trace.hpp"
#include "../win32/image.hpp"
#include "../win32/process.hpp"
// #include "../win32/loader.hpp"

namespace atom::memory
{

#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

std::uintptr_t ScanRegionInternal( const std::uint8_t* region_begin, const std::uint8_t* region_end, const char* signature )
{
	if( !region_begin )
	{
		TRACE( "%s: region_begin is nullptr!", __FUNCTION__ );
		return 0;
	}

	if( !region_end )
	{
		TRACE( "%s: region_end is nullptr!", __FUNCTION__ );
		return 0;
	}

	if( !signature )
	{
		TRACE( "%s: signature is nullptr!", __FUNCTION__ );
		return 0;
	}

	auto scan_result = static_cast< std::uintptr_t >( 0 );
	auto scan_compare = reinterpret_cast< const std::uint8_t* >( signature );

	const auto scan_begin = region_begin;
	const auto scan_end = region_end;

	for( auto scan_current = scan_begin; scan_current < scan_end; scan_current++ )
	{
		if( is_terminator( scan_compare[ 0 ] ) )
			return scan_result;

		if( MmIsAddressValid( const_cast< std::uint8_t* >( scan_current ) ) )
		{
			if( is_question( scan_compare[ 0 ] ) || scan_current[ 0 ] == to_byte( scan_compare ) )
			{
				if( !scan_result )
					scan_result = ToAddress( scan_current );

				if( is_terminator( scan_compare[ 2 ] ) )
					return scan_result;

				const bool question[ 2 ] =
				{
					is_question( scan_compare[ 0 ] ),
					is_question( scan_compare[ 1 ] ),
				};

				if( ( question[ 0 ] && question[ 1 ] ) || ( !question[ 0 ] ) )
					scan_compare = ( scan_compare + 3 );
				else
					scan_compare = ( scan_compare + 2 );
			}
			else
			{
				scan_compare = reinterpret_cast< const std::uint8_t* >( signature );
				scan_result = 0;
			}
		}
	}

	TRACE( "%s: Can't find \"%s\" signature in [0x%016llX]->[0x%016llX]!", __FUNCTION__, signature, ToAddress( scan_begin ), ToAddress( scan_end ) );
	return 0;
}

std::uintptr_t ScanImageSectionInternal( std::uintptr_t image, const char* name, const char* signature )
{
	if( !image )
	{
		TRACE( "%s: image is 0!", __FUNCTION__ );
		return 0;
	}

	if( !name )
	{
		TRACE( "%s: name is nullptr!", __FUNCTION__ );
		return 0;
	}

	const auto image_section = win32::GetImageSection( image, name );

	if( !image_section )
	{
		TRACE( "%s: win32::GetImageSection( 0x%016llX, \"%s\" ) error!", __FUNCTION__, image, name );
		return 0;
	}

	const auto region_begin = reinterpret_cast< const std::uint8_t* >( image + image_section->VirtualAddress );
	const auto region_end = reinterpret_cast< const std::uint8_t* >( image + image_section->VirtualAddress + image_section->Misc.VirtualSize );

	return ScanRegionInternal( region_begin, region_end, signature );
}

void* find_signature( void* memory, size_t size, const char* pattern, const char* mask )
{
	size_t sig_length = strlen( mask );
	if( sig_length > size ) return nullptr;

	for( size_t i = 0; i < size - sig_length; i++ )
	{
		bool found = true;
		for( size_t j = 0; j < sig_length; j++ )
			found &= mask[ j ] == '?' || pattern[ j ] == *( ( char* )memory + i + j );

		if( found )
			return ( char* )memory + i;
	}
	return nullptr;
}

std::uintptr_t ScanImageInternal( std::uintptr_t image, const char* signature )
{
	if( !image )
		return 0;

	const auto image_nt_headers = win32::GetImageNtHeaders( image );

	if( !image_nt_headers )
		return 0;

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	if( !image_section_header )
		return 0;

	for( std::uint16_t i = 0; i < image_nt_headers->FileHeader.NumberOfSections; i++ )
	{
		const auto image_section = &image_section_header[ i ];

		const auto region_begin = reinterpret_cast< const std::uint8_t* >( image + image_section->VirtualAddress );
		const auto region_end = reinterpret_cast< const std::uint8_t* >( image + image_section->VirtualAddress + image_section->Misc.VirtualSize );

		char image_section_name[ 9 ] = { };
		std::memcpy( image_section_name, image_section->Name, 8 );

		// TRACE( "%s: [0x%016llX][0x%016llX]", image_section_name, ToAddress( region_begin ), ToAddress( region_end ) );
		auto result = ScanRegionInternal( region_begin, region_end, signature );
		
		if( result )
		{
			return result;
		}
	}

	return 0;
}

std::uintptr_t ScanSectionInternal( const char* name, const char* signature )
{
	const auto image_ntoskrnl = win32::GetSystemImage( HASH( L"ntoskrnl.exe" ) );

	if( !image_ntoskrnl )
	{
		TRACE( "%s: win32::GetImageKernelEntry() error!", __FUNCTION__ );
		return 0;
	}

	return ScanImageSectionInternal( image_ntoskrnl, name, signature );
}

std::uintptr_t ScanInternal( const char* signature )
{
	const auto image_ntoskrnl = win32::GetSystemImage( HASH( L"ntoskrnl.exe" ) );

	if( !image_ntoskrnl )
		return 0;

	return ScanImageInternal( image_ntoskrnl, signature );
}

std::uintptr_t RipInternal( std::uintptr_t address, std::size_t byte_count )
{
	if( !address )
		return 0;

	const auto displacement = *reinterpret_cast< std::int32_t* >( address + byte_count );

	if( !displacement )
		return 0;

	return ( address + byte_count + displacement + sizeof( displacement ) );
}

}