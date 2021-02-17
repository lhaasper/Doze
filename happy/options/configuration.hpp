#pragma once

#include <chrono>

using namespace std::chrono_literals;

namespace atom
{

enum GameType
{
	GamePUBG,
	GamePUBGLite,
	GameRust,
};

namespace detail
{
constexpr const wchar_t* cx_game_class_names[] =
{
	L"UnityWndClass",
	L"UnrealWindow",
	L"UnityWndClass",
};
constexpr const wchar_t* cx_game_image_names[] =
{
	L"GameOverlayRenderer64.dll",
	L"nvapi64.dll",
	L"GameOverlayRenderer64.dll",
};
constexpr const wchar_t* cx_game_file_names[] =
{
	L"rust.dll",
	L"pubg-lite.dll",
	L"rust.dll",
};
} // namespace detail

constexpr auto cx_game_type = GameRust;
constexpr auto cx_game_class_name = detail::cx_game_class_names[ cx_game_type ];
constexpr auto cx_game_image_name = detail::cx_game_image_names[ cx_game_type ];
constexpr auto cx_game_file_name = detail::cx_game_file_names[ cx_game_type ];
constexpr auto cx_game_time_out = 5min;

} // namespace atom