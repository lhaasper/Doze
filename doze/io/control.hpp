#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "packet.hpp"

namespace atom::io
{

NTSTATUS DeviceControl( DEVICE_OBJECT* device_object, IRP* irp );
NTSTATUS DeviceControlComplete( IRP* irp, NTSTATUS status, ULONG_PTR information );

} // namespace atom::io