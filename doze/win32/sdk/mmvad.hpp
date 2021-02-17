#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "mmpte.hpp"

#define MM_ZERO_ACCESS				0
#define MM_READONLY						1
#define MM_EXECUTE						2
#define MM_EXECUTE_READ				3
#define MM_READWRITE					4
#define MM_WRITECOPY					5
#define MM_EXECUTE_READWRITE	6
#define MM_EXECUTE_WRITECOPY	7

namespace atom::win32
{

struct MM_AVL_NODE
{
	MM_AVL_NODE* LeftChild = nullptr;
	MM_AVL_NODE* RightChild = nullptr;
	union
	{
		struct
		{
			std::int64_t Balance : 2;
		};
		MM_AVL_NODE* Parent = nullptr;
	} u1 = { };
};

struct RTL_AVL_TREE
{
	MM_AVL_NODE* BalancedRoot = nullptr;
	void* NodeHint = nullptr;
	std::uint64_t NumberGenericTableElements = 0;
};

struct EX_PUSH_LOCK
{
	union
	{
		struct
		{
			std::uint64_t Locked : 1;
			std::uint64_t Waiting : 1;
			std::uint64_t Waking : 1;
			std::uint64_t MultipleShared : 1;
			std::uint64_t Shared : 60;
		};
		std::uint64_t Value;
		void* Ptr = nullptr;
	};
};

struct MMVAD_FLAGS
{
	std::uint32_t VadType : 3;
	std::uint32_t Protection : 5;
	std::uint32_t PreferredNode : 6;
	std::uint32_t NoChange : 1;
	std::uint32_t PrivateMemory : 1;
	std::uint32_t Teb : 1;
	std::uint32_t PrivateFixup : 1;
	std::uint32_t ManySubsections : 1;
	std::uint32_t Spare : 12;
	std::uint32_t DeleteInProgress : 1;
};

struct MMVAD_FLAGS1
{
	std::uint32_t CommitCharge : 31;
	std::uint32_t MemCommit : 1;
};

struct MMVAD_FLAGS2
{
	std::uint32_t FileOffset : 24;
	std::uint32_t Large : 1;
	std::uint32_t TrimBehind : 1;
	std::uint32_t Inherit : 1;
	std::uint32_t CopyOnWrite : 1;
	std::uint32_t NoValidationNeeded : 1;
	std::uint32_t Spare : 3;
};

struct MI_VAD_SEQUENTIAL_INFO
{
	std::uint64_t Length : 12;
	std::uint64_t Vpn : 52;
};

struct MMVAD_SHORT
{
	union
	{
		RTL_BALANCED_NODE VadNode = { };
		MMVAD_SHORT* NextVad;
	};
	std::uint32_t StartingVpn = 0;
	std::uint32_t EndingVpn = 0;
	std::uint8_t StartingVpnHigh = 0;
	std::uint8_t EndingVpnHigh = 0;
	std::uint8_t CommitChargeHigh = 0;
	std::uint8_t SpareNT64VadUChar = 0;
	std::int32_t ReferenceCount = 0;
	EX_PUSH_LOCK PushLock = { };
	union
	{
		std::uint32_t LongFlags = 0;
		MMVAD_FLAGS VadFlags;
	} u = { };
	union
	{
		std::uint32_t LongFlags1 = 0;
		MMVAD_FLAGS1 VadFlags1;
	} u1 = { };
	void* EventList = nullptr;
};

struct MMVAD
{
	MMVAD_SHORT Core = { };
	union
	{
		std::uint32_t LongFlags2 = 0;
		MMVAD_FLAGS2 VadFlags2;
	} u2 = { };
	std::uint32_t Padding = 0;
	void* Subsection = nullptr;
	MMPTE* FirstPrototypePte = nullptr;
	MMPTE* LastContiguousPte = nullptr;
	LIST_ENTRY ViewLinks = { };
	PEPROCESS VadsProcess = nullptr;
	struct
	{
		MI_VAD_SEQUENTIAL_INFO SequentialVa = { };
		void* ExtendedInfo;
	} u4 = { };
	FILE_OBJECT* FileObject = nullptr;
};

#define GET_VAD_ROOT( Table )		Table->BalancedRoot

}