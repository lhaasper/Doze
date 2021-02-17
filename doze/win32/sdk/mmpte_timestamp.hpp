#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_TIMESTAMP
{
	struct
	{
		std::uint64_t MustBeZero : 1;
		std::uint64_t Unused : 3;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t Transition : 1;
		std::uint64_t PageFileLow : 4;
		std::uint64_t Reserved : 16;
		std::uint64_t GlobalTimeStamp : 32;
	};
};

}