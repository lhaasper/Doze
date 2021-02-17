#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_PROTOTYPE
{
	struct
	{
		std::uint64_t Valid : 1;
		std::uint64_t DemandFillProto : 1;
		std::uint64_t HiberVerifyConverted : 1;
		std::uint64_t ReadOnly : 1;
		std::uint64_t SwizzleBit : 1;
		std::uint64_t Protection : 5;
		std::uint64_t Prototype : 1;
		std::uint64_t Combined : 1;
		std::uint64_t Unused1 : 4;
		std::int64_t ProtoAddress : 48;
	};
};

}