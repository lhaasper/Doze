#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#include "mmpte_hardware.hpp"
#include "mmpte_prototype.hpp"
#include "mmpte_software.hpp"
#include "mmpte_timestamp.hpp"
#include "mmpte_transition.hpp"
#include "mmpte_subsection.hpp"
#include "mmpte_list.hpp"

#define PFN_TO_PAGE( PFn )		( ( PFn ) << 12 )
#define PAGE_TO_PFN( Page )		( ( Page ) >> 12 )
#define PFN_TO_PDP_PAE( PFn )	( ( PFn ) << 5 )

namespace atom::win32
{

struct MMPTE
{
	union
	{
		union
		{
			std::uint64_t Long = 0;
			volatile std::uint64_t VolatileLong;
			MMPTE_HARDWARE Hard;
			MMPTE_PROTOTYPE Proto;
			MMPTE_SOFTWARE Soft;
			MMPTE_TIMESTAMP TimeStamp;
			MMPTE_TRANSITION Trans;
			MMPTE_SUBSECTION Subsect;
			MMPTE_LIST List;
		};
	} u = { };
};

}