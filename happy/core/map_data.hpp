#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../constant/hash.hpp"

namespace atom::core
{

constexpr std::uint32_t BadSystemNumber = std::numeric_limits< std::uint32_t >::max();

struct MapContext
{
	LARGE_INTEGER m_time_expired = { };	// 0x0000
};
// sizeof( MapContext ) = 0x0008

struct MapData
{
	static constexpr auto Signature = HASH( "MapData" );

	inline bool IsValid() const
	{
		return ( m_signature == Signature );
	}

	std::uint64_t m_signature = 0;

	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;

	LARGE_INTEGER m_time_expired = { };

	// 
	// native imports
	// 
	std::uintptr_t RtlAddFunctionTable = 0;
	std::uintptr_t RtlDeleteFunctionTable = 0;

	std::uintptr_t RtlAllocateHeap = 0;
	std::uintptr_t RtlReAllocateHeap = 0;
	std::uintptr_t RtlFreeHeap = 0;

	std::uintptr_t RtlQueryPerformanceCounter = 0;
	std::uintptr_t RtlQueryPerformanceFrequency = 0;

	std::uintptr_t RtlInitializeCriticalSection = 0;
	std::uintptr_t RtlInitializeCriticalSectionEx = 0;
	std::uintptr_t RtlEnterCriticalSection = 0;
	std::uintptr_t RtlTryEnterCriticalSection = 0;
	std::uintptr_t RtlLeaveCriticalSection = 0;
	std::uintptr_t RtlDeleteCriticalSection = 0;

	std::uintptr_t RtlInitializeConditionVariable = 0;
	std::uintptr_t RtlSleepConditionVariableCS = 0;
	std::uintptr_t RtlWakeConditionVariable = 0;
	std::uintptr_t RtlWakeAllConditionVariable = 0;

	std::uintptr_t __C_specific_handler = 0;

	// 
	// kernel32 imports
	// 
	std::uintptr_t OutputDebugStringA = 0;
	std::uintptr_t OutputDebugStringW = 0;

	// 
	// msvcrt imports
	// 
	std::uintptr_t malloc = 0;
	std::uintptr_t free = 0;

	std::uintptr_t memchr = 0;
	std::uintptr_t memcmp = 0;
	std::uintptr_t memcpy = 0;
	std::uintptr_t memmove = 0;
	std::uintptr_t memset = 0;

	std::uintptr_t qsort = 0;
	std::uintptr_t strstr = 0;

	std::uintptr_t _vsnprintf_l = 0;
	std::uintptr_t _vsnwprintf_l = 0;

	std::uintptr_t fabs = 0;
	std::uintptr_t fmod = 0;
	std::uintptr_t pow = 0;
	std::uintptr_t atan2 = 0;
	std::uintptr_t ceil = 0;
	std::uintptr_t floor = 0;
	std::uintptr_t atol = 0;
	std::uintptr_t atof = 0;

	std::uintptr_t sscanf = 0;

	// 
	// objects
	// 
	std::uint32_t NtClose = BadSystemNumber;
	std::uint32_t NtDuplicateObject = BadSystemNumber;
	std::uint32_t NtMakeTemporaryObject = BadSystemNumber;
	std::uint32_t NtQueryObject = BadSystemNumber;
	std::uint32_t NtSetInformationObject = BadSystemNumber;
	std::uint32_t NtSignalAndWaitForSingleObject = BadSystemNumber;
	std::uint32_t NtWaitForMultipleObjects = BadSystemNumber;
	std::uint32_t NtWaitForSingleObject = BadSystemNumber;

	// 
	// events
	// 
	std::uint32_t NtClearEvent = BadSystemNumber;
	std::uint32_t NtCreateEvent = BadSystemNumber;
	std::uint32_t NtOpenEvent = BadSystemNumber;
	std::uint32_t NtPulseEvent = BadSystemNumber;
	std::uint32_t NtResetEvent = BadSystemNumber;
	std::uint32_t NtSetEvent = BadSystemNumber;

	// 
	// files
	// 
	std::uint32_t NtCreateFile = BadSystemNumber;
	std::uint32_t NtReadFile = BadSystemNumber;
	std::uint32_t NtWriteFile = BadSystemNumber;
	std::uint32_t NtLockFile = BadSystemNumber;
	std::uint32_t NtUnlockFile = BadSystemNumber;
	std::uint32_t NtDeleteFile = BadSystemNumber;
	std::uint32_t NtQueryInformationFile = BadSystemNumber;
	std::uint32_t NtQueryDirectoryFile = BadSystemNumber;

	// 
	// memory
	// 
	std::uint32_t NtAllocateVirtualMemory = BadSystemNumber;
	std::uint32_t NtFreeVirtualMemory = BadSystemNumber;
	std::uint32_t NtLockVirtualMemory = BadSystemNumber;
	std::uint32_t NtUnlockVirtualMemory = BadSystemNumber;
	std::uint32_t NtQueryVirtualMemory = BadSystemNumber;
	std::uint32_t NtProtectVirtualMemory = BadSystemNumber;
	std::uint32_t NtReadVirtualMemory = BadSystemNumber;
	std::uint32_t NtWriteVirtualMemory = BadSystemNumber;

	// 
	// win32 user
	// 
	std::uint32_t NtUserGetKeyNameText = BadSystemNumber;
	std::uint32_t NtUserGetKeyState = BadSystemNumber;
	std::uint32_t NtUserGetAsyncKeyState = BadSystemNumber;
	std::uint32_t NtUserMapVirtualKeyEx = BadSystemNumber;
};

} // namespace atom::core

extern LARGE_INTEGER g_time_expire;
// extern atom::core::MapContext* g_map_context;