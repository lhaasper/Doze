#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct IMAGE_FILE_HEADER
{
	std::uint16_t Machine = 0;
	std::uint16_t NumberOfSections = 0;
	std::uint32_t TimeDateStamp = 0;
	std::uint32_t PointerToSymbolTable = 0;
	std::uint32_t NumberOfSymbols = 0;
	std::uint16_t SizeOfOptionalHeader = 0;
	std::uint16_t Characteristics = 0;
};

}