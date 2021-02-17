#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#define DEVICE_NULL												L"\\Device\\Null"
#define DEVICE_BEEP												L"\\Device\\Beep"
#define DEVICE_KSECDD											L"\\Device\\KsecDD"
#define DEVICE_LANMAN_DATAGRAM_RECEIVER		L"\\Device\\LanmanDatagramReceiver"

namespace atom
{

extern DRIVER_OBJECT* g_driver_object;
extern DRIVER_DISPATCH* g_driver_dispatch;

NTSTATUS HookDispatch( const wchar_t* const object_name );
NTSTATUS UnhookDispatch();

bool IsDispatchHooked();

} // namespace atom