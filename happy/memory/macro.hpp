#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#define PAGE_SIZE					( 4096 )

#define FIELD_PAD( size )	std::uint8_t JOIN( __pad, __COUNTER__ )[ size ] = { }

namespace atom::memory
{

} // namespace atom::memory