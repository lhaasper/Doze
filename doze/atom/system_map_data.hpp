#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../constant/hash.hpp"

namespace atom
{

struct SystemMapData
{
	bool IsValid() const;

	std::uint64_t m_signature = 0;

	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;

	struct
	{
		std::uintptr_t PsLoadedModuleList = 0;
		std::uintptr_t PsLoadedModuleResource = 0;
		std::uintptr_t PsLoadedModuleSpinLock = 0;
		std::uintptr_t PsInvertedFunctionTable = 0;

		std::uintptr_t MmUnloadedDrivers = 0;
		std::uintptr_t MmLastUnloadedDriver = 0;

		std::uintptr_t PiDDBCacheList = 0;
		std::uintptr_t PiDDBCacheTable = 0;
		std::uintptr_t PiDDBLock = 0;
		std::uintptr_t PiDDBPath = 0;

		std::uintptr_t RtlInsertInvertedFunctionTable = 0;

		std::uintptr_t NtQueryVirtualMemory = 0;
		std::uintptr_t NtProtectVirtualMemory = 0;
		std::uintptr_t NtAllocateVirtualMemory = 0;
		std::uintptr_t NtFreeVirtualMemory = 0;
		std::uintptr_t NtReadVirtualMemory = 0;
		std::uintptr_t NtWriteVirtualMemory = 0;
		std::uintptr_t NtLockVirtualMemory = 0;
		std::uintptr_t NtUnlockVirtualMemory = 0;
	} m_symbols = { };
};

} // namespace atom

extern atom::SystemMapData g_map_data;