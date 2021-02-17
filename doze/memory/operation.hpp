#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::memory
{

constexpr std::size_t PointerSize = sizeof( std::uintptr_t );

constexpr std::uintptr_t MinimumUserAddress = 0x0000000000010000;
constexpr std::uintptr_t MaximumUserAddress = 0x000EFFFFFFFFFFFF;

constexpr std::uintptr_t MinimumSystemAddress = 0x8000000000000000;

std::uintptr_t ToAddress( const void* pointer ) noexcept;
void* ToPointer( std::uintptr_t address ) noexcept;
const void* ToConstantPointer( std::uintptr_t address ) noexcept;

bool IsUserAddress( std::uintptr_t address ) noexcept;
bool IsUserAddress( const void* pointer ) noexcept;

bool IsSystemAddress( std::uintptr_t address ) noexcept;
bool IsSystemAddress( const void* pointer ) noexcept;

bool IsAddressValid( std::uintptr_t address ) noexcept;
bool IsAddressValid( const void* pointer ) noexcept;

} // namespace atom::memory