#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "image_data_directory.hpp"

namespace atom::win32
{

struct IMAGE_OPTIONAL_HEADER32
{
	std::uint16_t Magic = 0;
	std::uint8_t MajorLinkerVersion = 0;
	std::uint8_t MinorLinkerVersion = 0;
	std::uint32_t SizeOfCode = 0;
	std::uint32_t SizeOfInitializedData = 0;
	std::uint32_t SizeOfUninitializedData = 0;
	std::uint32_t AddressOfEntryPoint = 0;
	std::uint32_t BaseOfCode = 0;
	std::uint32_t ImageBase = 0;
	std::uint32_t SectionAlignment = 0;
	std::uint32_t FileAlignment = 0;
	std::uint16_t MajorOperatingSystemVersion = 0;
	std::uint16_t MinorOperatingSystemVersion = 0;
	std::uint16_t MajorImageVersion = 0;
	std::uint16_t MinorImageVersion = 0;
	std::uint16_t MajorSubsystemVersion = 0;
	std::uint16_t MinorSubsystemVersion = 0;
	std::uint32_t Win32VersionValue = 0;
	std::uint32_t SizeOfImage = 0;
	std::uint32_t SizeOfHeaders = 0;
	std::uint32_t CheckSum = 0;
	std::uint16_t Subsystem = 0;
	std::uint16_t DllCharacteristics = 0;
	std::uint32_t SizeOfStackReserve = 0;
	std::uint32_t SizeOfStackCommit = 0;
	std::uint32_t SizeOfHeapReserve = 0;
	std::uint32_t SizeOfHeapCommit = 0;
	std::uint32_t LoaderFlags = 0;
	std::uint32_t NumberOfRvaAndSizes = 0;
	IMAGE_DATA_DIRECTORY DataDirectory[ IMAGE_NUMBEROF_DIRECTORY_ENTRIES ] = { };
};

struct IMAGE_OPTIONAL_HEADER64
{
	std::uint16_t Magic = 0;
	std::uint8_t MajorLinkerVersion = 0;
	std::uint8_t MinorLinkerVersion = 0;
	std::uint32_t SizeOfCode = 0;
	std::uint32_t SizeOfInitializedData = 0;
	std::uint32_t SizeOfUninitializedData = 0;
	std::uint32_t AddressOfEntryPoint = 0;
	std::uint32_t BaseOfCode = 0;
	std::uint64_t ImageBase = 0;
	std::uint32_t SectionAlignment = 0;
	std::uint32_t FileAlignment = 0;
	std::uint16_t MajorOperatingSystemVersion = 0;
	std::uint16_t MinorOperatingSystemVersion = 0;
	std::uint16_t MajorImageVersion = 0;
	std::uint16_t MinorImageVersion = 0;
	std::uint16_t MajorSubsystemVersion = 0;
	std::uint16_t MinorSubsystemVersion = 0;
	std::uint32_t Win32VersionValue = 0;
	std::uint32_t SizeOfImage = 0;
	std::uint32_t SizeOfHeaders = 0;
	std::uint32_t CheckSum = 0;
	std::uint16_t Subsystem = 0;
	std::uint16_t DllCharacteristics = 0;
	std::uint64_t SizeOfStackReserve = 0;
	std::uint64_t SizeOfStackCommit = 0;
	std::uint64_t SizeOfHeapReserve = 0;
	std::uint64_t SizeOfHeapCommit = 0;
	std::uint32_t LoaderFlags = 0;
	std::uint32_t NumberOfRvaAndSizes = 0;
	IMAGE_DATA_DIRECTORY DataDirectory[ IMAGE_NUMBEROF_DIRECTORY_ENTRIES ] = { };
};

#if defined( HORIZON_X32 )
using IMAGE_OPTIONAL_HEADER = IMAGE_OPTIONAL_HEADER32;
#elif defined( HORIZON_X64 )
using IMAGE_OPTIONAL_HEADER = IMAGE_OPTIONAL_HEADER64;
#endif // HORIZON_X32

}