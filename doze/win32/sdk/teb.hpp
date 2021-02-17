#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "list_entry.hpp"
#include "nt_tib.hpp"
#include "client_id.hpp"
#include "peb.hpp"

namespace atom::win32
{

struct ACTIVATION_CONTEXT_STACK
{
	void* ActiveFrame = nullptr;
	LIST_ENTRY FrameListCache = { };
	std::uint32_t Flags = 0;
	std::uint32_t NextCookieSequenceNumber = 0;
	std::uint32_t StackId = 0;
	FIELD_PAD( 0x0004 );
};

struct TEB
{
	NT_TIB NtTib = { };
	void* EnvironmentPointer = nullptr;
	CLIENT_ID ClientId = { };
	void* ActiveRpcHandle = nullptr;
	void* ThreadLocalStoragePointer = nullptr;
	PEB* ProcessEnvironmentBlock = nullptr;
	std::uint32_t LastErrorValue = 0;
	std::uint32_t CountOfOwnedCriticalSections = 0;
	void* CsrClientThread = nullptr;
	void* Win32ThreadInfo = nullptr;
	std::uint32_t User32Reserved[ 26 ] = { };
	std::uint32_t UserReserved[ 5 ] = { };
	FIELD_PAD( 0x0004 );
	void* WOW32Reserved = nullptr;
	std::uint32_t CurrentLocale = 0;
	std::uint32_t FpSoftwareStatusRegister = 0;
	void* ReservedForDebuggerInstrumentation[ 16 ] = { };
	void* SystemReserved1[ 30 ] = { };
	std::int8_t PlaceholderCompatibilityMode = 0;
	std::uint8_t PlaceholderHydrationAlwaysExplicit = 0;
	std::int8_t PlaceholderReserved[ 10 ] = { };
	std::uint32_t ProxiedProcessId = 0;
	ACTIVATION_CONTEXT_STACK _ActivationStack = { };
	std::uint8_t WorkingOnBehalfTicket[ 8 ] = { };
	std::int32_t ExceptionCode = 0;
	FIELD_PAD( 0x0004 );
};

}