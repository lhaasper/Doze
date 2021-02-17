#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES			16

#define IMAGE_DIRECTORY_ENTRY_EXPORT					0
#define IMAGE_DIRECTORY_ENTRY_IMPORT					1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE				2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION				3
#define IMAGE_DIRECTORY_ENTRY_SECURITY				4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC				5
#define IMAGE_DIRECTORY_ENTRY_DEBUG						6
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE		7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR				8
#define IMAGE_DIRECTORY_ENTRY_TLS							9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG			10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT		11
#define IMAGE_DIRECTORY_ENTRY_IAT							12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT		13
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

namespace atom::win32
{

struct IMAGE_DATA_DIRECTORY
{
	std::uint32_t VirtualAddress = 0;
	std::uint32_t Size = 0;
};

}