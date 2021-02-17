#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct MMPTE_HARDWARE
{
	std::uint64_t Valid : 1;
	std::uint64_t Dirty1 : 1;
	std::uint64_t Owner : 1;
	std::uint64_t WriteThrough : 1;
	std::uint64_t CacheDisable : 1;
	std::uint64_t Accessed : 1;
	std::uint64_t Dirty : 1;
	std::uint64_t LargePage : 1;
	std::uint64_t Global : 1;
	std::uint64_t CopyOnWrite : 1;
	std::uint64_t Unused : 1;
	std::uint64_t Write : 1;
	std::uint64_t PageFrameNumber : 36;
	std::uint64_t ReservedForHardware : 4;
	std::uint64_t ReservedForSoftware : 4;
	std::uint64_t WsleAge : 4;
	std::uint64_t WsleProtection : 3;
	std::uint64_t NoExecute : 1;
};

}