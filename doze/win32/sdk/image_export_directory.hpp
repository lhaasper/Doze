#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct IMAGE_EXPORT_DIRECTORY
{
	std::uint32_t Characteristics = 0;
	std::uint32_t TimeDateStamp = 0;
	std::uint16_t MajorVersion = 0;
	std::uint16_t MinorVersion = 0;
	std::uint32_t Name = 0;
	std::uint32_t Base = 0;
	std::uint32_t NumberOfFunctions = 0;
	std::uint32_t NumberOfNames = 0;
	std::uint32_t AddressOfFunctions = 0;
	std::uint32_t AddressOfNames = 0;
	std::uint32_t AddressOfNameOrdinals = 0;
};

}