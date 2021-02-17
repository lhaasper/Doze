#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct LIST_ENTRY
{
	LIST_ENTRY* Flink = nullptr;	// 0x0000
	LIST_ENTRY* Blink = nullptr;	// 0x0008
};
// sizeof( LIST_ENTRY ) = 0x0010

} // namespace atom::win32