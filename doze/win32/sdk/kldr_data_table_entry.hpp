#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "list_entry.hpp"
#include "unicode_string.hpp"

namespace atom::win32
{

struct KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks = { };				// 0x0000
	void* ExceptionTable = nullptr;						// 0x0010
	std::uint32_t ExceptionTableSize = 0;			// 0x0018
	FIELD_PAD( 0x0004 );											// 0x001C
	void* GpValue = nullptr;									// 0x0020
	void* NonPagedDebugInfo = nullptr;				// 0x0028
	void* DllBase = nullptr;									// 0x0030
	void* EntryPoint = nullptr;								// 0x0038
	std::uint32_t SizeOfImage = 0;						// 0x0040
	FIELD_PAD( 0x0004 );											// 0x0044
	UNICODE_STRING FullDllName = { };					// 0x0048
	UNICODE_STRING BaseDllName = { };					// 0x0058
	std::uint32_t Flags = 0;									// 0x0068
	std::uint16_t LoadCount = 0;							// 0x006C
	union
	{
		std::uint16_t EntireField = 0;					// 0x006E
		struct
		{
			std::uint16_t SignatureLevel : 4;			// 0x006E
			std::uint16_t SignatureType : 3;			// 0x006E
			std::uint16_t Unused : 9;							// 0x006E
		};
	};
	void* SectionPointer = nullptr;						// 0x0070
	std::uint32_t CheckSum = 0;								// 0x0078
	std::uint32_t CoverageSectionSize = 0;		// 0x007C
	void* CoverageSection = nullptr;					// 0x0080
	void* LoadedImports = nullptr;						// 0x0088
	void* Spare = nullptr;										// 0x0090
	std::uint32_t SizeOfImageNotRounded = 0;	// 0x0098
	std::uint32_t TimeDateStamp = 0;					// 0x009C
};
// sizeof( KLDR_DATA_TABLE_ENTRY ) = 0x00A0

} // namespace atom::win32