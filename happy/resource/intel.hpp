#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

constexpr auto cx_intel_device = L"Nal";
constexpr auto cx_intel_driver = L"iqvm64e.sys";

namespace atom::resource
{

extern std::array< std::uint8_t, 34568 > m_binary_intel;

} // namespace atom::resource