#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

union PML4E
{
	union
	{
		unsigned long long Value;
		struct
		{
			unsigned long long P : 1; // Present
			unsigned long long RW : 1; // Read/Write
			unsigned long long US : 1; // User/Supervisor
			unsigned long long PWT : 1; // Page-Level Writethrough
			unsigned long long PCD : 1; // Page-Level Cache Disable
			unsigned long long A : 1; // Accessed
			unsigned long long Ignored0 : 1;
			unsigned long long Reserved1 : 2;
			unsigned long long AVL : 3; // Available to software
			unsigned long long PDP : 40;
			unsigned long long Available : 11;
			unsigned long long NX : 1; // No Execute
		} Page4Kb, Page2Mb, Page1Gb, Generic; // Same for the 4Kb, 2Mb and 1Gb page size
	} x64;
};

}