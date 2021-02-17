#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

union CR3
{
	unsigned long long Value;
	union
	{
		unsigned int Value;
		struct
		{
			unsigned int Reserved0 : 3;
			unsigned int PWT : 1; // Page-Level Writethrough
			unsigned int PCD : 1; // Page-Level Cache Disable
			unsigned int Reserved1 : 7;
			unsigned int PD : 20; // Page-Directory-Table Base Address
		} NonPae;
		struct
		{
			unsigned int Reserved0 : 3;
			unsigned int PWT : 1; // Page-Level Writethrough
			unsigned int PCD : 1; // Page-Level Cache Disable
			unsigned int PDP : 27; // Page-Directory-Pointer-Table Base Address
		} Pae;
	} x32;
	union
	{
		unsigned long long Value;
		struct
		{
			unsigned long long Reserved0 : 3;
			unsigned long long PWT : 1; // Page-Level Writethrough
			unsigned long long PCD : 1; // Page-Level Cache Disable
			unsigned long long Reserved1 : 7;
			unsigned long long PML4 : 40; // Page-Map Level-4 Table Base Address
			unsigned long long Reserved2 : 12;
		} Bitmap;
	} x64;
};

}