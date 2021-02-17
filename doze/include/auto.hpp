#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )

#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>

#include <ntstrsafe.h>

#pragma warning( pop )

#include <memory.h>

using int8_t = signed __int8;
using int16_t = signed __int16;
using int32_t = signed __int32;
using int64_t = signed __int64;

#if defined( ATOM_X32 )
using intptr_t = int32_t;
#elif defined( ATOM_X64 )
using intptr_t = int64_t;
#endif // ATOM_X32

using uint8_t = unsigned __int8;
using uint16_t = unsigned __int16;
using uint32_t = unsigned __int32;
using uint64_t = unsigned __int64;

#if defined( ATOM_X32 )
using uintptr_t = uint32_t;
#elif defined( ATOM_X64 )
using uintptr_t = uint64_t;
#endif // ATOM_X32

using size_t = uintptr_t;
using ssize_t = intptr_t;

using ptrdiff_t = intptr_t;

namespace std
{

using ::int8_t;
using ::int16_t;
using ::int32_t;
using ::int64_t;

using ::intptr_t;

using ::uint8_t;
using ::uint16_t;
using ::uint32_t;
using ::uint64_t;

using ::uintptr_t;

using ::size_t;
using ::ssize_t;

using ::ptrdiff_t;

using ::memcpy;
using ::memmove;
using ::memset;

/*
using ::memchr;
using ::memcmp;
using ::memcpy;
using ::memcpy_s;
using ::memmove;
using ::memmove_s;
using ::memset;

using ::strlen;
using ::strcmp;
using ::strcpy;
using ::strcpy_s;
using ::strncpy;
using ::strncpy_s;
using ::strncmp;

using ::wcslen;
using ::wcscmp;
using ::wcscpy;
using ::wcscpy_s;
using ::wcsncpy;
using ::wcsncpy_s;
using ::wcsncmp;
*/

} // namespace std

// #define FIELD_PAD( Size )						std::uint8_t JOIN( __pad, __COUNTER__ )[ Size ] = { }