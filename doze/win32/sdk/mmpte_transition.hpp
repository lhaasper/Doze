#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_TRANSITION
{
	struct
	{
		std::uint64_t Valid : 1;
		std::uint64_t Write : 1;
		std::uint64_t Spare : 1;
		std::uint64_t IoTracker : 1;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t Transition : 1;
		std::uint64_t PageFrameNumber : 36;
		std::uint64_t Unused : 16;
	};
};

}