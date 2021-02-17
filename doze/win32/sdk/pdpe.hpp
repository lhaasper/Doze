#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

union PDPE
{
	union
	{
		union
		{
			unsigned long long Value;
			struct
			{
				unsigned long long P : 1; // Present
				unsigned long long Reserved0 : 2;
				unsigned long long PWT : 1; // Page-Level Writethrough
				unsigned long long PCD : 1; // Page-Level Cache Disable
				unsigned long long Reserved1 : 4;
				unsigned long long AVL : 3; // Available to software
				unsigned long long PD : 40;
				unsigned long long Reserved2 : 12;
			} Page4Kb, Page2Mb, Generic; // Same for the 4Kb and 2Mb page size
		} Pae;
	} x32;
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
			unsigned long long Reserved0 : 1;
			unsigned long long PS : 1; // PageSize == 0
			unsigned long long Reserved1 : 1;
			unsigned long long AVL : 3; // Available to software
			unsigned long long Reserved3 : 51;
			unsigned long long NX : 1; // No Execute
		} Generic;
		union
		{
			struct
			{
				unsigned long long P : 1; // Present
				unsigned long long RW : 1; // Read/Write
				unsigned long long US : 1; // User/Supervisor
				unsigned long long PWT : 1; // Page-Level Writethrough
				unsigned long long PCD : 1; // Page-Level Cache Disable
				unsigned long long A : 1; // Accessed
				unsigned long long Ignored0 : 1;
				unsigned long long PS : 1; // PageSize == 0
				unsigned long long Reserved0 : 1;
				unsigned long long AVL : 3; // Available to software
				unsigned long long PD : 40;
				unsigned long long Available : 11;
				unsigned long long NX : 1; // No Execute
			} Page4Kb, Page2Mb, Generic;
		} NonPageSize; // PDPE.PS == 0
		union
		{
			struct
			{
				unsigned long long P : 1; // Present
				unsigned long long RW : 1; // Read/Write
				unsigned long long US : 1; // User/Supervisor
				unsigned long long PWT : 1; // Page-Level Writethrough
				unsigned long long PCD : 1; // Page-Level Cache Disable
				unsigned long long A : 1; // Accessed
				unsigned long long D : 1; // Dirty
				unsigned long long PS : 1; // PageSize == 1
				unsigned long long G : 1; // Global Page
				unsigned long long AVL : 3; // Available to software
				unsigned long long PAT : 1; // Page-Attribute Table
				unsigned long long Reserved0 : 17;
				unsigned long long PhysicalPageBase : 22;
				unsigned long long Available : 11;
				unsigned long long NX : 1; // No Execute
			} Page1Gb;
		} PageSize; // PDPE.PS == 1
	} x64;
};

}