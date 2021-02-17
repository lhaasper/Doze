#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom
{

struct SymbolData
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
};

extern SymbolData g_symbols;

} // namespace atom