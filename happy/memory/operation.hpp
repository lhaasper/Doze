#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#define PAGE_SIZE				0x1000

#define HORIZON_PAD( Size )		std::uint8_t HORIZON_JOIN( __pad, __COUNTER__ )[ Size ] = { }

namespace atom::memory
{

constexpr std::uint32_t MinimumUserAddress32 = 0x00010000;
constexpr std::uint32_t MaximumUserAddress32 = 0x7FFFFFFF;

constexpr std::uint64_t MinimumUserAddress64 = 0x0000000000010000;
constexpr std::uint64_t MaximumUserAddress64 = 0x00007FFFFFFFFFFF;

#if defined( HORIZON_X32 )
constexpr auto MinimumUserAddress = MinimumUserAddress32;
constexpr auto MaximumUserAddress = MaximumUserAddress32;
#elif defined( HORIZON_X64 )
constexpr auto MinimumUserAddress = MinimumUserAddress64;
constexpr auto MaximumUserAddress = MaximumUserAddress64;
#endif // HORIZON_X32

inline std::uintptr_t ToAddress( const void* pointer )
{
	return reinterpret_cast< std::uintptr_t >( pointer );
}

inline void* ToPointer( std::uintptr_t address )
{
	return reinterpret_cast< void* >( address );
}

inline const void* ToConstantPointer( std::uintptr_t address )
{
	return reinterpret_cast< const void* >( address );
}

inline bool IsUserAddress( std::uintptr_t address )
{
	return ( address >= MinimumUserAddress &&
					 address <= MaximumUserAddress );
}

inline bool IsUserAddress( const void* pointer )
{
	const auto address = ToAddress( pointer );
	return IsUserAddress( address );
}

inline bool IsAddressValid( std::uintptr_t address )
{
	return IsUserAddress( address );
}

inline bool IsAddressValid( const void* pointer )
{
	return IsUserAddress( pointer );
}

} // namespace atom::memory