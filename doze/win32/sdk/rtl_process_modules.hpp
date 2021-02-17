#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct RTL_PROCESS_MODULE_INFORMATION
{
	void* Section = nullptr;
	void* MappedBase = nullptr;
	void* ImageBase = nullptr;
	std::uint32_t ImageSize = 0;
	std::uint32_t Flags = 0;
	std::uint16_t LoadOrderIndex = 0;
	std::uint16_t InitOrderIndex = 0;
	std::uint16_t LoadCount = 0;
	std::uint16_t OffsetToFileName = 0;
	std::uint8_t FullPathName[ MAXIMUM_FILENAME_LENGTH ] = { };
};

struct RTL_PROCESS_MODULES
{
	std::uint32_t NumberOfModules = 0;
	RTL_PROCESS_MODULE_INFORMATION Modules[ 1 ] = { };
};

}