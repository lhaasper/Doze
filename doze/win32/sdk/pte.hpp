#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

union PTE
{
	union
	{
		union
		{
			unsigned int Value;
			struct
			{
				unsigned int P : 1; // Present
				unsigned int RW : 1; // Read/Write
				unsigned int US : 1; // User/Supervisor
				unsigned int PWT : 1; // Page-Level Writethrough
				unsigned int PCD : 1; // Page-Level Cache Disable
				unsigned int A : 1; // Accessed
				unsigned int D : 1; // Dirty
				unsigned int PAT : 1; // Page-Attribute Table
				unsigned int G : 1; // Global Page
				unsigned int AVL : 3; // Available to software
				unsigned int PhysicalPageBase : 20;
			} Page4Kb;
		} NonPae;
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
				unsigned long long D : 1; // Dirty
				unsigned long long PAT : 1; // Page-Attribute Table
				unsigned long long G : 1; // Global Page
				unsigned long long AVL : 3; // Available to software
				unsigned long long PhysicalPageBase : 40;
				unsigned long long Reserved0 : 11;
				unsigned long long NX : 1; // No Execute
			} Page4Kb;
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
			unsigned long long D : 1; // Dirty
			unsigned long long PAT : 1; // Page-Attribute Table
			unsigned long long G : 1; // Global Page
			unsigned long long AVL : 3; // Available to software
			unsigned long long PhysicalPageBase : 40;
			unsigned long long Available : 11;
			unsigned long long NX : 1; // No Execute
		} Page4Kb;
	} x64;
};

}