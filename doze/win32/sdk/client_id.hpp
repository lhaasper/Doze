#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct CLIENT_ID
{
	void* UniqueProcess = nullptr;
	void* UniqueThread = nullptr;
};

}