#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

struct NT_TIB
{
	void* ExceptionList = nullptr;
	void* StackBase = nullptr;
	void* StackLimit = nullptr;
	void* SubSystemTib = nullptr;
	union
	{
		void* FiberData = nullptr;
		std::uint32_t Version;
	};
	void* ArbitraryUserPointer = nullptr;
	NT_TIB* Self = nullptr;
};

}