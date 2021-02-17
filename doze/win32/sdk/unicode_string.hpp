#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "../../memory/macro.hpp"

namespace atom::win32
{

struct UNICODE_STRING
{
	std::uint16_t Length = 0;					// 0x0000
	std::uint16_t MaximumLength = 0;	// 0x0002
	FIELD_PAD( 0x0004 );							// 0x0004
	wchar_t* Buffer = nullptr;				// 0x0008
};
// sizeof( UNICODE_STRING ) = 0x0010

} // namespace atom::win32