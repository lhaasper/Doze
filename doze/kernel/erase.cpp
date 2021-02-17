#include "erase.hpp"
#include "import.hpp"

#include "../atom/system_map_data.hpp"

#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../win32/image.hpp"
#include "../win32/trace.hpp"
#include "../win32/critical_region.hpp"
#include "../win32/resource_lite.hpp"

namespace kernel
{

using namespace atom;

NTSTATUS QueryMmUnloadedDrivers( PUNLOADED_DRIVER* MmUnloadedDrivers )
{
	auto status = STATUS_SUCCESS;

	if( !g_map_data.IsValid() )
	{
		return status = STATUS_INVALID_PARAMETER;
	}

	*MmUnloadedDrivers = *reinterpret_cast< PUNLOADED_DRIVER* >( g_map_data.m_symbols.MmUnloadedDrivers );

	return status;
}

NTSTATUS QueryMmLastUnloadedDriver( PULONG* MmLastUnloadedDriver )
{
	auto status = STATUS_SUCCESS;

	if( !g_map_data.IsValid() )
	{
		return status = STATUS_INVALID_PARAMETER;
	}

	*MmLastUnloadedDriver = reinterpret_cast< PULONG >( g_map_data.m_symbols.MmLastUnloadedDriver );

	return status;
}

NTSTATUS EraseMmUnloadedDrivers( const UNICODE_STRING& erase_name )
{
	auto status = STATUS_SUCCESS;

	if( !erase_name.Length )
	{
		TRACE( "%s: erase_name is empty!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER;
	}

	auto mm_unloaded_drivers = PUNLOADED_DRIVER( nullptr );

	status = QueryMmUnloadedDrivers( &mm_unloaded_drivers );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: QueryMmUnloadedDrivers( 0x%016llX ) error! (0x%08X)", __FUNCTION__, std::uintptr_t( &mm_unloaded_drivers ), status );
		return status;
	}

	auto mm_last_unloaded_driver = PULONG( nullptr );

	status = QueryMmLastUnloadedDriver( &mm_last_unloaded_driver );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: QueryMmLastUnloadedDriver( 0x%016llX ) error! (0x%08X)", __FUNCTION__, std::uintptr_t( &mm_last_unloaded_driver ), status );
		return status;
	}

	auto mm_unloaded_drivers_erased = PUNLOADED_DRIVER( ExAllocatePoolWithTag( NonPagedPoolNx, MI_UNLOADED_DRIVERS * sizeof( UNLOADED_DRIVER ), 'TDmM' ) );
	auto mm_last_unloaded_driver_erased = 0ul;

	if( !mm_unloaded_drivers_erased )
	{
		TRACE( "%s: ExAllocatePoolWithTag( NonPagedPoolNx, 0x%016llX, 0x%08X ) error!", __FUNCTION__, MI_UNLOADED_DRIVERS * sizeof( UNLOADED_DRIVER ), 'TDmM' );
		return status = STATUS_INSUFFICIENT_RESOURCES;
	}

	std::memset( mm_unloaded_drivers_erased, 0, MI_UNLOADED_DRIVERS * sizeof( UNLOADED_DRIVER ) );

	win32::CriticalRegion region = { };
	win32::CriticalRegionGuard region_guard( region );

	const auto PsLoadedModuleResource = const_cast< ERESOURCE* >( win32::GetPsLoadedModuleResource() );

	win32::ResourceLite resource( PsLoadedModuleResource );
	win32::ResourceLiteGuard resource_guard( resource, true );

	for( auto i = 0; i < MI_UNLOADED_DRIVERS; i++ )
	{
		const auto unloaded_driver = &mm_unloaded_drivers[ i ];
		const auto unloaded_driver_erased = &mm_unloaded_drivers_erased[ mm_last_unloaded_driver_erased ];

		if( !unloaded_driver )
			continue;

		if( !unloaded_driver_erased )
			continue;

		if( !unloaded_driver->Name.Length )
			continue;

		if( RtlCompareUnicodeString( &unloaded_driver->Name, &erase_name, TRUE ) == 0 )
		{
			TRACE( "%s: Erasing \"%wZ\" from MmUnloadedDrivers!", __FUNCTION__, unloaded_driver->Name );
			continue;
		}

		std::memcpy( unloaded_driver_erased, unloaded_driver, sizeof( UNLOADED_DRIVER ) );
		
		mm_last_unloaded_driver_erased++;
	}

	std::memcpy( mm_unloaded_drivers, mm_unloaded_drivers_erased, MI_UNLOADED_DRIVERS * sizeof( UNLOADED_DRIVER ) );
	std::memcpy( mm_last_unloaded_driver, &mm_last_unloaded_driver_erased, sizeof( ULONG ) );

	ExFreePoolWithTag( mm_unloaded_drivers_erased, 'TDmM' );
	return status;
}

NTSTATUS QueryPiDDBLock( PERESOURCE* PiDDBLock )
{
	auto status = STATUS_SUCCESS;

	if( !g_map_data.IsValid() )
	{
		return status = STATUS_INVALID_PARAMETER;
	}

	*PiDDBLock = reinterpret_cast< PERESOURCE >( g_map_data.m_symbols.PiDDBLock );

	return status;
}

NTSTATUS QueryPiDDBCacheTable( PRTL_AVL_TABLE* PiDDBCacheTable )
{
	auto status = STATUS_SUCCESS;

	if( !g_map_data.IsValid() )
	{
		return status = STATUS_INVALID_PARAMETER;
	}

	*PiDDBCacheTable = reinterpret_cast< PRTL_AVL_TABLE >( g_map_data.m_symbols.PiDDBCacheTable );

	return status;
}

NTSTATUS ErasePiDDBCacheEntry( PRTL_AVL_TABLE pi_ddb_cache_table, PPI_DDB_CACHE_ENTRY lookup_cache_entry )
{
	auto status = STATUS_SUCCESS;

	if( !pi_ddb_cache_table )
	{
		TRACE( "%s: pi_ddb_cache_table is nullptr!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER;
	}

	if( !lookup_cache_entry )
	{
		TRACE( "%s: lookup_cache_entry is nullptr!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER;
	}

	auto cache_entry = PPI_DDB_CACHE_ENTRY( RtlLookupElementGenericTableAvl( pi_ddb_cache_table, lookup_cache_entry ) );

	if( !cache_entry )
	{
		TRACE( "%s: RtlLookupElementGenericTableAvl( 0x%016llX, 0x%016llX ) error!", __FUNCTION__, std::uintptr_t( pi_ddb_cache_table ), std::uintptr_t( lookup_cache_entry ) );
		return status = STATUS_NOT_FOUND;
	}

	RemoveEntryList( &cache_entry->ListEntry );

	if( !RtlDeleteElementGenericTableAvl( pi_ddb_cache_table, cache_entry ) )
	{
		TRACE( "%s: RtlDeleteElementGenericTableAvl( 0x%016llX, 0x%016llX ) error!", __FUNCTION__, std::uintptr_t( pi_ddb_cache_table ), std::uintptr_t( cache_entry ) );
		return status = STATUS_UNSUCCESSFUL;
	}

	TRACE( "%s: Erased (\"%wZ\", 0x%08X) from PiDDBCacheTable!", __FUNCTION__, lookup_cache_entry->DriverName, lookup_cache_entry->TimeDateStamp );
	return status;
}

NTSTATUS ErasePiDDBCacheTable( const UNICODE_STRING& erase_name, ULONG time_date_stamp )
{
	auto nt_status = STATUS_SUCCESS;

	if( !erase_name.Length )
	{
		TRACE( "%s: erase_name is empty!", __FUNCTION__ );
		return nt_status = STATUS_INVALID_PARAMETER;
	}

	auto pi_ddb_lock = PERESOURCE( nullptr );

	nt_status = QueryPiDDBLock( &pi_ddb_lock );

	if( !NT_SUCCESS( nt_status ) )
	{
		TRACE( "%s: QueryPiDDBLock( 0x%016llX ) error! (0x%08X)", __FUNCTION__, std::uintptr_t( &pi_ddb_lock ), nt_status );
		return nt_status;
	}

	auto pi_ddb_cache_table = PRTL_AVL_TABLE( nullptr );

	nt_status = QueryPiDDBCacheTable( &pi_ddb_cache_table );

	if( !NT_SUCCESS( nt_status ) )
	{
		TRACE( "%s: QueryPiDDBCacheTable( 0x%016llX ) error! (0x%08X)", __FUNCTION__, std::uintptr_t( &pi_ddb_cache_table ), nt_status );
		return nt_status;
	}

	PI_DDB_CACHE_ENTRY erase_cache_entry = { };

	erase_cache_entry.DriverName = erase_name;
	erase_cache_entry.TimeDateStamp = time_date_stamp;
	erase_cache_entry.LoadStatus = STATUS_SUCCESS;

	win32::CriticalRegion region = { };
	win32::CriticalRegionGuard region_guard( region );

	win32::ResourceLite resource( pi_ddb_lock );
	win32::ResourceLiteGuard resource_guard( resource, true );

	if( RtlIsGenericTableEmptyAvl( pi_ddb_cache_table ) )
	{
		TRACE( "%s: RtlIsGenericTableEmptyAvl( 0x%016llX ) error!", __FUNCTION__, std::uintptr_t( pi_ddb_cache_table ) );
		return nt_status = STATUS_UNSUCCESSFUL;
	}

	nt_status = ErasePiDDBCacheEntry( pi_ddb_cache_table, &erase_cache_entry );

	if( !NT_SUCCESS( nt_status ) )
		TRACE( "%s: ErasePiDDBCacheEntry( 0x%016llX, 0x%016llX ) error! (0x%08X)", __FUNCTION__, std::uintptr_t( pi_ddb_cache_table ), std::uintptr_t( &erase_cache_entry ), nt_status );

	return nt_status;
}

} // namespace kernel