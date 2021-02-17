#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/singleton.hpp"

namespace atom::win32
{

class Console : public core::Singleton< Console >
{
public:
	~Console();

public:
	bool Create( const std::wstring& caption );
	void Destroy();

public:
	void LogV( const char* format, va_list args );
	void LogV( const wchar_t* format, va_list args );

public:
	template< typename T >
	void Log( const T* format, ... )
	{
		va_list args = { };
		va_start( args, format );
		LogV( format, args );
		va_end( args );
	}

protected:
	std::uint32_t m_process_id = 0;
	std::wstring m_caption = { };

	FILE* m_std_input = nullptr;
	FILE* m_std_output = nullptr;
	FILE* m_std_error = nullptr;

	HANDLE m_std_input_handle = INVALID_HANDLE_VALUE;
	HANDLE m_std_output_handle = INVALID_HANDLE_VALUE;
	HANDLE m_std_error_handle = INVALID_HANDLE_VALUE;
	
	// FILE* m_conin_stdin = nullptr;
	// FILE* m_conout_stdout = nullptr;
	// FILE* m_conout_stderr = nullptr;
};

} // namespace atom::win32

#if defined( HORIZON_DEBUG )
#define HORIZON_LOG( Format, ... )		atom::win32::Console::Instance().Log( Format, __VA_ARGS__ )
#else
#define HORIZON_LOG( Format, ... )		atom::win32::Console::Instance().Log( Format, __VA_ARGS__ )
// #define HORIZON_LOG( ... )						( __VA_ARGS__ )
#endif

#define HORIZON_LOG_SEPARATOR()				HORIZON_LOG( "============================================================" )

#define TRACE													HORIZON_LOG
#define TRACE_SEPARATOR()							HORIZON_LOG_SEPARATOR()