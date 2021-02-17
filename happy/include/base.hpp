#pragma once

#if defined( _M_IX86 )
	#define HORIZON_X32
#elif defined( _M_AMD64 )
	#define HORIZON_X64
#endif // _M_IX86

#if defined( _DEBUG )
	#define HORIZON_DEBUG
#elif defined( NDEBUG )
	//#define HORIZON_DEBUG
	#define HORIZON_RELEASE
#endif // _DEBUG

#define HORIZON_WARNING_PUSH( Warning )		\
	__pragma( warning( push ) )				\
	__pragma( warning( disable : Warning ) )\

#define HORIZON_WARNING_POP()				\
	__pragma( warning( pop ) )

#define HORIZON_IMPORT( Name )				\
	__pragma( comment( lib, Name ) )

#define API_FORCEINLINE					__forceinline

#define API_CDECL						__cdecl
#define API_STDCALL						__stdcall
#define API_THISCALL					__thiscall
#define API_FASTCALL					__fastcall

#define API_NT							API_STDCALL
#define API_WIN32						API_STDCALL
#define API_D3D							API_STDCALL

#define API_IMPORT						__declspec( dllimport )
#define API_EXPORT						__declspec( dllexport )

#define API_NORETURN					__declspec( noreturn )

#define ATOM_FUNCTION					__FUNCTION__

#define HORIZON_EXTERN						extern "C"
#define HORIZON_BEGIN_EXTERN			extern "C" {
#define HORIZON_END_EXTERN				}

#define HORIZON_JOIN_IMPL( A, B )		A ## B
#define HORIZON_JOIN( A, B )				HORIZON_JOIN_IMPL( A, B )

constexpr bool g_use_debug_console = true;
constexpr bool g_use_debug_image = true;
constexpr bool g_use_debug_doze = false;
constexpr bool g_use_debug_doze_load = false;
constexpr bool g_use_debug_exception = false;