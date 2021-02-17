#pragma once

#if !defined( WIN32_LEAN_AND_MEAN )
	#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#if !defined( NOMINMAX )
	#define NOMINMAX
#endif // !NOMINMAX

#include <windows.h>
#include <winternl.h>
#include <winioctl.h>

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <ntstatus.h>
#pragma warning( pop )

#include <psapi.h>
#include <tlhelp32.h>

#define WordSize sizeof(void*)

#define LAST_STATUS_OFS (0x598 + 0x197 * WordSize)

inline NTSTATUS LastNtStatus()
{
	return *( NTSTATUS* )( ( unsigned char* )NtCurrentTeb() + LAST_STATUS_OFS );
}

inline NTSTATUS SetLastNtStatus( NTSTATUS status )
{
	return *( NTSTATUS* )( ( unsigned char* )NtCurrentTeb() + LAST_STATUS_OFS ) = status;
}