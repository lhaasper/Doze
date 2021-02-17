#include "dialog.hpp"
#include "debug_trace.hpp"

namespace atom::win32
{

const std::uint32_t g_dialog_button[] =
{
	MB_OK,
	MB_OK | MB_ICONWARNING,
	MB_OK | MB_ICONERROR,
};

const std::string g_dialog_caption[] =
{
	"[Message]",
	"[Warning]",
	"[Error]",
};

const std::wstring g_dialog_caption_wide[] =
{
	L"[Message]",
	L"[Warning]",
	L"[Error]",
};

int DialogExecute( const char* caption, const char* message, std::uint32_t button )
{
	return MessageBoxA( nullptr, message, caption, UINT( button ) );
}

int DialogExecute( const wchar_t* caption, const wchar_t* message, std::uint32_t button )
{
	return MessageBoxW( nullptr, message, caption, UINT( button ) );
}

int DialogV( std::uint32_t dialog_type, const char* format, va_list args )
{
	const auto button = g_dialog_button[ dialog_type ];
	const auto caption = g_dialog_caption[ dialog_type ];

	char output[ 4096 ] = { };
	vsprintf_s( output, format, args );

	const auto code = DialogExecute( caption.c_str(), output, button );
	win32::TraceFormat( format, args );
	return code;
}

int DialogV( std::uint32_t dialog_type, const wchar_t* format, va_list args )
{
	const auto button = g_dialog_button[ dialog_type ];
	const auto caption = g_dialog_caption_wide[ dialog_type ];

	wchar_t output[ 4096 ] = { };
	vswprintf_s( output, format, args );

	const auto code = DialogExecute( caption.c_str(), output, button );
	win32::TraceFormat( format, args );
	return code;
}

}