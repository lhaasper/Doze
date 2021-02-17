#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom
{

bool DumpDeviceDrivers();

std::uintptr_t GetDeviceDriver( const std::wstring& name );

}