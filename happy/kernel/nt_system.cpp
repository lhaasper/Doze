#include "nt_system.hpp"
#include "loader.hpp"

namespace atom::kernel
{

NtSystem::NtSystem( Loader& loader )
	: m_loader( loader )
{ }

int NtSystem::memcmp( const void* left, const void* right, std::size_t size )
{
	int result = 0;
	Call( "memcmp", &result, left, right, size );
	return result;
}

void* NtSystem::memcpy( void* destination, const void* source, std::size_t size )
{
	void* result = nullptr;
	Call( "memcpy", &result, destination, source, size );
	return result;
}

void* NtSystem::memmove( void* destination, const void* source, std::size_t size )
{
	void* result = nullptr;
	Call( "memmove", &result, destination, source, size );
	return result;
}

void* NtSystem::memset( void* destination, int value, std::size_t size )
{
	void* result = nullptr;
	Call( "memset", &result, destination, value, size );
	return result;
}

std::uint32_t NtSystem::RtlRandom( std::uint32_t* seed )
{
	std::uint32_t result = 0;
	Call( "RtlRandom", &result, seed );
	return result;
}

std::uint32_t NtSystem::RtlRandomEx( std::uint32_t* seed )
{
	std::uint32_t result = 0;
	Call( "RtlRandomEx", &result, seed );
	return result;
}

void* NtSystem::ExAllocatePool( _POOL_TYPE pool_type, std::size_t number_of_bytes )
{
	void* result = nullptr;
	Call( "ExAllocatePool", &result, pool_type, number_of_bytes );
	return result;
}

void* NtSystem::ExAllocatePoolWithTag( POOL_TYPE pool_type, std::size_t number_of_bytes, std::uint32_t tag )
{
	void* result = nullptr;
	Call( "ExAllocatePoolWithTag", &result, pool_type, number_of_bytes, tag );
	return result;
}

void NtSystem::ExFreePool( void* pool )
{
	Call< void >( "ExFreePool", nullptr, pool );
}

void NtSystem::ExFreePoolWithTag( void* pool, std::uint32_t tag )
{
	Call< void >( "ExFreePoolWithTag", nullptr, pool, tag );
}

template< typename T, typename... ArgsT >
FORCEINLINE bool NtSystem::Call( const std::string& name, T* result, ArgsT... args )
{
	return m_loader.Call< T >( name, result, args... );
}

}