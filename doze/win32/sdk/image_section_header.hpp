#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

#define IMAGE_FIRST_SECTION( ImageNtHeaders )																						\
	( ( const atom::win32::IMAGE_SECTION_HEADER* )( std::uintptr_t( ImageNtHeaders ) +	\
		FIELD_OFFSET( atom::win32::IMAGE_NT_HEADERS, OptionalHeader ) +									\
		( ( ImageNtHeaders ) )->FileHeader.SizeOfOptionalHeader ) )

struct IMAGE_SECTION_HEADER
{
	std::uint8_t Name[ 8 ] = { };
	union
	{
		std::uint32_t PhysicalAddress;
		std::uint32_t VirtualSize;
	} Misc = { };
	std::uint32_t VirtualAddress = 0;
	std::uint32_t SizeOfRawData = 0;
	std::uint32_t PointerToRawData = 0;
	std::uint32_t PointerToRelocations = 0;
	std::uint32_t PointerToLinenumbers = 0;
	std::uint16_t NumberOfRelocations = 0;
	std::uint16_t NumberOfLinenumbers = 0;
	std::uint32_t Characteristics = 0;
};

}