#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "list_entry.hpp"
#include "unicode_string.hpp"

namespace atom::win32
{

enum LDR_DLL_LOAD_REASON
{
	LoadReasonStaticDependency = 0,
	LoadReasonStaticForwarderDependency = 1,
	LoadReasonDynamicForwarderDependency = 2,
	LoadReasonDelayloadDependency = 3,
	LoadReasonDynamicLoad = 4,
	LoadReasonAsImageLoad = 5,
	LoadReasonAsDataLoad = 6,
	LoadReasonEnclavePrimary = 7,
	LoadReasonEnclaveDependency = 8,
	LoadReasonUnknown = -1,
};

#pragma warning( push )
#pragma warning( disable : 4201 )

struct LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks = { };
	LIST_ENTRY InMemoryOrderLinks = { };
	LIST_ENTRY InInitializationOrderLinks = { };
	void* DllBase = nullptr;
	void* EntryPoint = nullptr;
	std::uint32_t SizeOfImage = 0;
	FIELD_PAD( 0x0004 );
	UNICODE_STRING FullDllName = { };
	UNICODE_STRING BaseDllName = { };
	union
	{
		std::uint8_t FlagGroup[ 4 ] = { };
		std::uint32_t Flags;
		struct
		{
			std::uint32_t PackagedBinary : 1; /* bit position: 0 */
			std::uint32_t MarkedForRemoval : 1; /* bit position: 1 */
			std::uint32_t ImageDll : 1; /* bit position: 2 */
			std::uint32_t LoadNotificationsSent : 1; /* bit position: 3 */
			std::uint32_t TelemetryEntryProcessed : 1; /* bit position: 4 */
			std::uint32_t ProcessStaticImport : 1; /* bit position: 5 */
			std::uint32_t InLegacyLists : 1; /* bit position: 6 */
			std::uint32_t InIndexes : 1; /* bit position: 7 */
			std::uint32_t ShimDll : 1; /* bit position: 8 */
			std::uint32_t InExceptionTable : 1; /* bit position: 9 */
			std::uint32_t ReservedFlags1 : 2; /* bit position: 10 */
			std::uint32_t LoadInProgress : 1; /* bit position: 12 */
			std::uint32_t LoadConfigProcessed : 1; /* bit position: 13 */
			std::uint32_t EntryProcessed : 1; /* bit position: 14 */
			std::uint32_t ProtectDelayLoad : 1; /* bit position: 15 */
			std::uint32_t ReservedFlags3 : 2; /* bit position: 16 */
			std::uint32_t DontCallForThreads : 1; /* bit position: 18 */
			std::uint32_t ProcessAttachCalled : 1; /* bit position: 19 */
			std::uint32_t ProcessAttachFailed : 1; /* bit position: 20 */
			std::uint32_t CorDeferredValidate : 1; /* bit position: 21 */
			std::uint32_t CorImage : 1; /* bit position: 22 */
			std::uint32_t DontRelocate : 1; /* bit position: 23 */
			std::uint32_t CorILOnly : 1; /* bit position: 24 */
			std::uint32_t ChpeImage : 1; /* bit position: 25 */
			std::uint32_t ReservedFlags5 : 2; /* bit position: 26 */
			std::uint32_t Redirected : 1; /* bit position: 28 */
			std::uint32_t ReservedFlags6 : 2; /* bit position: 29 */
			std::uint32_t CompatDatabaseProcessed : 1; /* bit position: 31 */
		};
	};
	std::uint16_t ObsoleteLoadCount = 0;
	std::uint16_t TlsIndex = 0;
	LIST_ENTRY HashLinks = { };
	std::uint32_t TimeDateStamp = 0;
	FIELD_PAD( 0x0004 );
	void* EntryPointActivationContext = nullptr;
	void* Lock = nullptr;
	void* DdagNode = nullptr;
	LIST_ENTRY NodeModuleLink = { };
	void* LoadContext = nullptr;
	void* ParentDllBase = nullptr;
	void* SwitchBackContext = nullptr;
	RTL_BALANCED_NODE BaseAddressIndexNode = { };
	RTL_BALANCED_NODE MappingInfoIndexNode = { };
	std::uint64_t OriginalBase = 0;
	LARGE_INTEGER LoadTime = { };
	std::uint32_t BaseNameHashValue = 0;
	LDR_DLL_LOAD_REASON LoadReason = LoadReasonUnknown;
	std::uint32_t ImplicitPathOptions = 0;
	std::uint32_t ReferenceCount = 0;
	std::uint32_t DependentLoadFlags = 0;
	std::uint8_t SigningLevel = 0;
	FIELD_PAD( 0x0003 );
};

#pragma warning( pop )

} // namespace atom::win32