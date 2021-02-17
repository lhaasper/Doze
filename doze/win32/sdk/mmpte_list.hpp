#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_LIST
{
	struct
	{
		std::uint64_t Valid : 1;
		std::uint64_t OneEntry : 1;
		std::uint64_t filler0 : 2;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t Transition : 1;
		std::uint64_t filler1 : 16;
		std::uint64_t NextEntry : 36;
	};
};

}