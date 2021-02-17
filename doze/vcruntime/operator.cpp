#include "operator.hpp"

#include "../memory/macro.hpp"
#include "../memory/operation.hpp"

namespace atom::vcruntime
{

extern "C"
{

	void* API_STDCALL ExAllocatePoolWithTag( win32::POOL_TYPE PoolType,
																					 std::size_t NumberOfBytes,
																					 std::uint32_t Tag );

	void API_STDCALL ExFreePoolWithTag( void* BaseAddress,
																			std::uint32_t Tag );

}; // extern "C"

constexpr std::uint32_t Tag = 'TDmM';

void* AllocatePool( win32::POOL_TYPE pool_type, std::size_t number_of_bytes )
{
	return ExAllocatePoolWithTag( pool_type, number_of_bytes, Tag );
}

void FreePool( void* base_address )
{
	ExFreePoolWithTag( base_address, Tag );
}

void* AllocateMemory( std::size_t number_of_bytes )
{
	if( !number_of_bytes )
	{
		number_of_bytes = 1;
	}

	auto data = AllocatePool( win32::NonPagedPool, number_of_bytes );

	if( data )
	{
		std::memset( data, 0, number_of_bytes );
	}

	return data;
}

void FreeMemory( void* data )
{
	if( data )
	{
		FreePool( data );
	}
}

} // namespace atom::vcruntime

void* operator new( std::size_t size )
{
	return atom::vcruntime::AllocateMemory( size );
}

void* operator new[]( std::size_t size )
{
	return atom::vcruntime::AllocateMemory( size );
}

void operator delete( void* data )
{
	atom::vcruntime::FreeMemory( data );
}

void operator delete( void* data, std::size_t )
{
	atom::vcruntime::FreeMemory( data );
}

void operator delete[]( void* data )
{
	atom::vcruntime::FreeMemory( data );
}

void operator delete[]( void* data, std::size_t )
{
	atom::vcruntime::FreeMemory( data );
}