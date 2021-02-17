#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

// #include "base.hpp"

namespace atom::win32
{

class CriticalRegion
{
public:
	CriticalRegion();

public:
	void Enter();
	void Leave();
};

class CriticalRegionGuard
{
public:
	CriticalRegionGuard( CriticalRegion& critical_region );
	~CriticalRegionGuard();

protected:
	CriticalRegion& m_critical_region;
};

}