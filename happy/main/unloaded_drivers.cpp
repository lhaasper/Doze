#include "unloaded_drivers.hpp"

#include "../memory/operation.hpp"

namespace atom
{

using memory::ToAddress;
using memory::ToPointer;

bool DumpUnloadedDrivers( kernel::Loader& loader )
{
	auto& system = loader.GetNtSystem();

	const auto mm_unloaded_drivers_address = loader.Scan( "4C 8B 15 ? ? ? ? 4C 8B C9" );

	if( !mm_unloaded_drivers_address )
	{
		HORIZON_LOG( "%s: mm_unloaded_drivers_address is 0!", __FUNCTION__ );
		return false;
	}

	const auto mm_unloaded_drivers_pointer = loader.Rip( mm_unloaded_drivers_address, 3 );

	if( !mm_unloaded_drivers_pointer )
	{
		HORIZON_LOG( "%s: mm_unloaded_drivers_pointer is 0!", __FUNCTION__ );
		return false;
	}

	const auto mm_unloaded_drivers_location = loader.ReadUInt64( mm_unloaded_drivers_pointer );

	if( !mm_unloaded_drivers_location )
	{
		HORIZON_LOG( "%s: mm_unloaded_drivers_location is 0!", __FUNCTION__ );
		return false;
	}

	std::vector< UNLOADED_DRIVER > mm_unloaded_drivers( MI_UNLOADED_DRIVERS );
	mm_unloaded_drivers.resize( MI_UNLOADED_DRIVERS );

	if( !loader.ReadMemory( mm_unloaded_drivers_location, mm_unloaded_drivers.data(), sizeof( UNLOADED_DRIVER[ MI_UNLOADED_DRIVERS ] ) ) )
	{
		HORIZON_LOG( "%s: ReadMemory( ... ) error!", __FUNCTION__ );
		return false;
	}

	for( int i = 0; i < MI_UNLOADED_DRIVERS; i++ )
	{
		const auto& unloaded_driver = mm_unloaded_drivers[ i ];

		if( !unloaded_driver.Valid() )
			continue;

		const auto driver_name_data = ToAddress( unloaded_driver.Name.Buffer );
		const auto driver_name_size = static_cast< std::size_t >( unloaded_driver.Name.Length );
		const auto driver_name_length = driver_name_size / sizeof( wchar_t );

		std::wstring driver_name( driver_name_length + 1, L'\0' );

		if( !loader.ReadMemory( driver_name_data, driver_name.data(), driver_name_size ) )
			continue;

		if( driver_name.empty() )
			continue;

		const auto driver_base = ToAddress( unloaded_driver.StartAddress );
		const auto driver_size = ToAddress( unloaded_driver.EndAddress ) - driver_base;

		HORIZON_LOG( "[%ws] 0x%016llX (0x%016llX)", driver_name.c_str(), driver_base, driver_size );
	}

	HORIZON_LOG_SEPARATOR();
	return !mm_unloaded_drivers.empty();
}

}