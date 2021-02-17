#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../kernel/loader.hpp"

#define MI_UNLOADED_DRIVERS 50

namespace atom
{

typedef struct _UNLOADED_DRIVER
{
	bool Valid() const
	{
		if( !Name.Length )
			return false;

		if( !Name.MaximumLength )
			return false;

		if( !Name.Buffer )
			return false;

		if( !StartAddress )
			return false;

		if( !EndAddress )
			return false;

		return true;
	}

	UNICODE_STRING Name = { };
	void* StartAddress = nullptr;
	void* EndAddress = nullptr;
	LARGE_INTEGER CurrentTime = { };
} UNLOADED_DRIVER, *PUNLOADED_DRIVER;

bool DumpUnloadedDrivers( kernel::Loader& loader );

}