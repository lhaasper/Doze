#include "device_drivers.hpp"

#include "../memory/operation.hpp"

#include "../win32/console.hpp"

namespace atom
{

using memory::ToAddress;
using memory::ToPointer;

std::size_t GetDeviceDriverCount()
{
	DWORD device_driver_size = 0;

	if( !K32EnumDeviceDrivers( nullptr, 0, &device_driver_size ) )
	{
		HORIZON_LOG( "%s: K32EnumDeviceDrivers( nullptr, 0, 0x%016llX ) error! (0x%08X)", __FUNCTION__, ToAddress( &device_driver_size ), GetLastError() );
		return 0;
	}

	if( !device_driver_size )
	{
		HORIZON_LOG( "%s: device_driver_size is 0!", __FUNCTION__ );
		return 0;
	}

	return static_cast< std::size_t >( device_driver_size ) / sizeof( void* );
}

bool GetDeviceDriverArray( void** device_driver_array, std::size_t device_driver_array_count )
{
	auto device_driver_array_size = static_cast< DWORD >( device_driver_array_count * sizeof( void* ) );

	if( !device_driver_array_size )
	{
		HORIZON_LOG( "%s: device_driver_array_size is 0!", __FUNCTION__ );
		return false;
	}

	DWORD device_driver_size = 0;

	if( !K32EnumDeviceDrivers( device_driver_array, device_driver_array_size, &device_driver_size ) )
	{
		HORIZON_LOG( "%s: K32EnumDeviceDrivers( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	return true;
}

bool DumpDeviceDrivers()
{
	const auto device_driver_count = GetDeviceDriverCount();

	if( !device_driver_count )
	{
		HORIZON_LOG( "%s: GetDeviceDriverCount() error!", __FUNCTION__ );
		return false;
	}

	std::vector< void* > device_driver_array( device_driver_count );
	device_driver_array.resize( device_driver_count );

	if( !GetDeviceDriverArray( device_driver_array.data(), device_driver_count ) )
	{
		HORIZON_LOG( "%s: GetDeviceDriverArray() error!", __FUNCTION__ );
		return false;
	}

	HORIZON_LOG_SEPARATOR();
	HORIZON_LOG( "[DumpDeviceDrivers]" );
	HORIZON_LOG_SEPARATOR();

	for( const auto& device_driver : device_driver_array )
	{
		std::wstring device_driver_name( MAX_PATH, L'\0' );
		
		if( !GetDeviceDriverBaseNameW( device_driver, device_driver_name.data(), MAX_PATH ) )
			continue;

		HORIZON_LOG( "[%ws] 0x%016llX", device_driver_name.c_str(), ToAddress( device_driver ) );
	}

	HORIZON_LOG_SEPARATOR();
	return !device_driver_array.empty();
}

}