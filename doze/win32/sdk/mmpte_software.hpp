#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_SOFTWARE
{
	struct
	{
		std::uint64_t Valid : 1;
		std::uint64_t PageFileReserved : 1;
		std::uint64_t PageFileAllocated : 1;
		std::uint64_t ColdPage : 1;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t Transition : 1;
		std::uint64_t PageFileLow : 4;
		std::uint64_t UsedPageTableEntries : 10;
		std::uint64_t ShadowStack : 1;
		std::uint64_t Unused : 5;
		std::uint64_t PageFileHigh : 32;
	};
};

}