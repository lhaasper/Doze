#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::memory
{

std::uintptr_t ScanData( const std::uint8_t* const data_begin,
												 const std::uint8_t* const data_end,
												 const char* const signature );

} // namespace atom::memory