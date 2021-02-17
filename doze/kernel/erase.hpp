#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../memory/macro.hpp"

#define MI_UNLOADED_DRIVERS		50

namespace atom::win32
{

struct MI_UNLOADED_DRIVER
{
	UNICODE_STRING BaseName = { };	// 0x0000
	void* Begin = nullptr;					// 0x0010
	void* End = nullptr;						// 0x0018
	LARGE_INTEGER SystemTime = { };	// 0x0020
};
// sizeof( MI_UNLOADED_DRIVER ) = 0x0028

struct PI_DDB_CACHE_ENTRY
{
	LIST_ENTRY Links = { };						// 0x0000
	UNICODE_STRING BaseName = { };		// 0x0010
	ULONG TimeDateStamp = 0;					// 0x0020
	NTSTATUS Status = STATUS_SUCCESS;	// 0x0024
	FIELD_PAD( 0x0010 );							// 0x0028
};
// sizeof( PI_DDB_CACHE_ENTRY ) = 0x0038

} // namespace atom::win32

typedef struct _UNLOADED_DRIVER
{
	UNICODE_STRING Name = { };
	PVOID StartAddress = nullptr;
	PVOID EndAddress = nullptr;
	LARGE_INTEGER CurrentTime = { };
} UNLOADED_DRIVER, *PUNLOADED_DRIVER;

typedef struct _PI_DDB_CACHE_ENTRY
{
	LIST_ENTRY ListEntry = { };
	UNICODE_STRING DriverName = { };
	ULONG TimeDateStamp = 0;
	NTSTATUS LoadStatus = STATUS_SUCCESS;
	FIELD_PAD( 0x0010 );
} PI_DDB_CACHE_ENTRY, *PPI_DDB_CACHE_ENTRY;

namespace kernel
{

constexpr const WCHAR cx_intel_driver[] = L"iqvm64e.sys";
constexpr const WCHAR cx_intel_device[] = L"Nal";

constexpr auto cx_intel_time_date_stamp = 0x5284EAC3;

NTSTATUS QueryMmUnloadedDrivers( PUNLOADED_DRIVER* MmUnloadedDrivers );
NTSTATUS QueryMmLastUnloadedDriver( PULONG* mm_last_unloaded_driver );

NTSTATUS EraseMmUnloadedDrivers( const UNICODE_STRING& erase_name );

NTSTATUS QueryPiDDBLock( PERESOURCE* pi_ddb_lock );
NTSTATUS QueryPiDDBCacheTable( PRTL_AVL_TABLE* pi_ddb_cache_table );

NTSTATUS ErasePiDDBCacheEntry( PRTL_AVL_TABLE pi_ddb_cache_table, PPI_DDB_CACHE_ENTRY lookup_cache_entry );
NTSTATUS ErasePiDDBCacheTable( const UNICODE_STRING& erase_name, ULONG time_date_stamp );

} // namespace kernel