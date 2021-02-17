#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "peb_ldr_data.hpp"

namespace atom::win32
{

struct PEB
{
	std::uint8_t InheritedAddressSpace = 0;
	std::uint8_t ReadImageFileExecOptions = 0;
	std::uint8_t BeingDebugged = 0;
	std::uint8_t BitField = 0;
	void* Mutant = nullptr;
	void* ImageBaseAddress = nullptr;
	PEB_LDR_DATA* Ldr = nullptr;
	void* ProcessParameters = nullptr;
	void* SubSystemData = nullptr;
	void* ProcessHeap = nullptr;
	void* FastPebLock = nullptr;
	void* AtlThunkSListPtr = nullptr;
	void* IFEOKey = nullptr;
	void* CrossProcessFlags = nullptr;
	void* KernelCallbackTable = nullptr;
	std::uint32_t SystemReserved = 0;
	std::uint32_t AtlThunkSListPtr32 = 0;
	void* ApiSetMap = nullptr;
};

}