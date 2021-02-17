#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

enum POOL_TYPE
{
	NonPagedPool = 0,
	NonPagedPoolExecute = 0,
	PagedPool = 1,
	NonPagedPoolMustSucceed = 2,
	DontUseThisType = 3,
	NonPagedPoolCacheAligned = 4,
	PagedPoolCacheAligned = 5,
	NonPagedPoolCacheAlignedMustS = 6,
	MaxPoolType = 7,
	NonPagedPoolBase = 0,
	NonPagedPoolBaseMustSucceed = 2,
	NonPagedPoolBaseCacheAligned = 4,
	NonPagedPoolBaseCacheAlignedMustS = 6,
	NonPagedPoolSession = 32,
	PagedPoolSession = 33,
	NonPagedPoolMustSucceedSession = 34,
	DontUseThisTypeSession = 35,
	NonPagedPoolCacheAlignedSession = 36,
	PagedPoolCacheAlignedSession = 37,
	NonPagedPoolCacheAlignedMustSSession = 38,
	NonPagedPoolNx = 512,
	NonPagedPoolNxCacheAligned = 516,
	NonPagedPoolSessionNx = 544,
};
// sizeof( POOL_TYPE ) = 0x0004

} // namespace atom::win32