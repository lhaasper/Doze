#pragma once

#if defined( ATOM_X32 )
#define API_CDECL										__cdecl
#define API_STDCALL									__stdcall
#define API_THISCALL								__thiscall
#define API_FASTCALL								__fastcall
#define API_VECTORCALL							__vectorcall
#elif defined( ATOM_X64 )
#define API_CDECL										__fastcall
#define API_STDCALL									__fastcall
#define API_THISCALL								__fastcall
#define API_FASTCALL								__fastcall
#define API_VECTORCALL							__fastcall
#endif // ATOM_X32

#define JOIN_IMPL( A, B )						A ## B
#define JOIN( A, B )								JOIN_IMPL( A, B )