#include "critical_region.hpp"

namespace atom::win32
{

CriticalRegion::CriticalRegion()
{ }

void CriticalRegion::Enter()
{
	KeEnterCriticalRegion();
}

void CriticalRegion::Leave()
{
	KeLeaveCriticalRegion();
}

CriticalRegionGuard::CriticalRegionGuard( CriticalRegion& critical_region )
	: m_critical_region( critical_region )
{
	m_critical_region.Enter();
}

CriticalRegionGuard::~CriticalRegionGuard()
{
	m_critical_region.Leave();
}

}