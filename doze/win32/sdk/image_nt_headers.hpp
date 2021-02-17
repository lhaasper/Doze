#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "image_file_header.hpp"
#include "image_optional_header.hpp"

namespace atom::win32
{

constexpr std::uint32_t ImageNtSignature = 0x00004550;

struct IMAGE_NT_HEADERS32
{
	std::uint32_t Signature = 0;
	IMAGE_FILE_HEADER FileHeader = { };
	IMAGE_OPTIONAL_HEADER32 OptionalHeader = { };
};

struct IMAGE_NT_HEADERS64
{
	std::uint32_t Signature = 0;
	IMAGE_FILE_HEADER FileHeader = { };
	IMAGE_OPTIONAL_HEADER64 OptionalHeader = { };
};

#if defined( ATOM_X32 )
using IMAGE_NT_HEADERS = IMAGE_NT_HEADERS32;
#elif defined( ATOM_X64 )
using IMAGE_NT_HEADERS = IMAGE_NT_HEADERS64;
#endif // ATOM_X32

} // namespace atom::win32