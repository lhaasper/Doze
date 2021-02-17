#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_SUBSECTION
{
	struct
	{
		std::uint64_t Valid : 1;
		std::uint64_t Unused0 : 3;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t ColdPage : 1;
		std::uint64_t Unused1 : 3;
		std::uint64_t ExecutePrivilege : 1;
		std::int64_t SubsectionAddress : 48;
	};
};

}