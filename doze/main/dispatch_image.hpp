#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/unicode_string.hpp"

namespace atom
{



void LoadImageNotifyRoutine( win32::UNICODE_STRING* FullImageName, HANDLE ProcessId, IMAGE_INFO* ImageInfo );

} // namespace atom