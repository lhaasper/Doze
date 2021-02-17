#include "scan.hpp"
#include "operation.hpp"

#include "../constant/character.hpp"

#define IN_RANGE( x, a, b )	( x >= a && x <= b )
#define GET_BITS( x )		( IN_RANGE( x, '0', '9' ) ? ( x - '0' ) : ( ( x & ( ~0x20 ) ) - 'A' + 0x0A ) )
#define GET_BYTE( x )		( GET_BITS( x[ 0 ] ) << 4 | GET_BITS( x[ 1 ] ) )

namespace atom::memory
{

std::uintptr_t ScanData( const std::uint8_t* const data_begin,
												 const std::uint8_t* const data_end,
												 const char* const signature )
{
	auto scan_result = ToAddress( nullptr );
	auto scan_compare = reinterpret_cast< const std::uint8_t* >( signature );

	const auto scan_begin = data_begin;
	const auto scan_end = data_end;

	for( auto scan_current = scan_begin; scan_current < scan_end; scan_current++ )
	{
		if( constant::IsQuestion( scan_compare[ 0 ] ) || scan_current[ 0 ] == GET_BYTE( scan_compare ) )
		{
			if( !scan_result )
			{
				scan_result = memory::ToAddress( scan_current );
			}

			if( constant::IsTerminator( scan_compare[ 2 ] ) )
			{
				return scan_result;
			}

			const bool question[ 2 ] =
			{
				constant::IsQuestion( scan_compare[ 0 ] ),
				constant::IsQuestion( scan_compare[ 1 ] ),
			};

			scan_compare += ( ( question[ 0 ] && question[ 1 ] ) || !question[ 0 ] ) ? 3 : 2;

			if( constant::IsTerminator( scan_compare[ 0 ] ) )
			{
				return scan_result;
			}
		}
		else
		{
			scan_result = 0;
			scan_compare = reinterpret_cast< const std::uint8_t* >( signature );
		}
	}

	return 0;
}

} // namespace atom::memory