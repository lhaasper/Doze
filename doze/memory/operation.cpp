#include "operation.hpp"

namespace atom::memory
{

std::uintptr_t ToAddress( const void* pointer ) noexcept
{
	return reinterpret_cast< std::uintptr_t >( pointer );
}

void* ToPointer( std::uintptr_t address ) noexcept
{
	return reinterpret_cast< void* >( address );
}

const void* ToConstantPointer( std::uintptr_t address ) noexcept
{
	return reinterpret_cast< const void* >( address );
}

bool IsUserAddress( std::uintptr_t address ) noexcept
{
	return ( address >= MinimumUserAddress &&
					 address <= MaximumUserAddress );
}

bool IsUserAddress( const void* pointer ) noexcept
{
	const auto address = ToAddress( pointer );
	return IsUserAddress( address );
}

bool IsSystemAddress( std::uintptr_t address ) noexcept
{
	return ( address >= MinimumSystemAddress );
}

bool IsSystemAddress( const void* pointer ) noexcept
{
	const auto address = ToAddress( pointer );
	return IsSystemAddress( address );
}

bool IsAddressValid( std::uintptr_t address ) noexcept
{
	const auto virtual_address = ToPointer( address );
	return ( MmIsAddressValid( virtual_address ) == TRUE );
}

bool IsAddressValid( const void* pointer ) noexcept
{
	const auto address = ToAddress( pointer );
	return IsAddressValid( address );
}

} // namespace atom::memory