#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/native.hpp"

namespace atom::kernel
{

class Loader;

class NtSystem
{
public:
	friend class Loader;

public:
	NtSystem( Loader& loader );

public:
	int memcmp( const void* left, const void* right, std::size_t size );
	void* memcpy( void* destination, const void* source, std::size_t size );
	void* memmove( void* destination, const void* source, std::size_t size );
	void* memset( void* destination, int value, std::size_t size );

public:
	std::uint32_t RtlRandom( std::uint32_t* seed );
	std::uint32_t RtlRandomEx( std::uint32_t* seed );

public:
	void* ExAllocatePool( _POOL_TYPE pool_type, std::size_t number_of_bytes );
	void* ExAllocatePoolWithTag( POOL_TYPE pool_type, std::size_t number_of_bytes, std::uint32_t tag );

	void ExFreePool( void* pool );
	void ExFreePoolWithTag( void* pool, std::uint32_t tag );

public:
	template< typename T, typename... ArgsT >
	bool Call( const std::string& name, T* result, ArgsT... args );

public:
	template< typename T = std::uintptr_t >
	T AllocatePool( POOL_TYPE pool_type, std::size_t number_of_bytes );

	template< typename T = std::uintptr_t >
	T AllocatePoolWithTag( POOL_TYPE pool_type, std::size_t number_of_bytes, std::uint32_t tag );

	template< typename T = std::uintptr_t >
	void FreePool( T pool );

	template< typename T = std::uintptr_t >
	void FreePoolWithTag( T pool, std::uint32_t tag );

	template< typename Type = std::uintptr_t >
	Type Allocate( std::size_t size )
	{
		return Type( ExAllocatePoolWithTag( NonPagedPool, size, 'TDmM' ) );
	}

	template< typename Type = std::uintptr_t >
	void Free( Type base_address )
	{
		ExFreePoolWithTag( PVOID( base_address ), 'TDmM' );
	}

protected:
	Loader& m_loader;
};

template< typename T /*= std::uintptr_t*/ >
FORCEINLINE T NtSystem::AllocatePool( POOL_TYPE pool_type, std::size_t number_of_bytes )
{
	return T( ExAllocatePool( pool_type, number_of_bytes ) );
}

template< typename T /*= std::uintptr_t*/ >
FORCEINLINE T NtSystem::AllocatePoolWithTag( POOL_TYPE pool_type, std::size_t number_of_bytes, std::uint32_t tag )
{
	return T( ExAllocatePoolWithTag( pool_type, number_of_bytes, tag ) );
}

template< typename T /*= std::uintptr_t*/ >
FORCEINLINE void NtSystem::FreePool( T pool )
{
	ExFreePool( PVOID( pool ) );
}

template< typename T /*= std::uintptr_t*/ >
FORCEINLINE void NtSystem::FreePoolWithTag( T pool, std::uint32_t tag )
{
	ExFreePoolWithTag( PVOID( pool ), tag );
}

}