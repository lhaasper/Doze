#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::win32
{

class ResourceLite
{
public:
	ResourceLite( PERESOURCE resource );

public:
	bool IsValid() const;

public:
	bool AcquireShared( bool wait );
	bool AcquireExclusive( bool wait );
	void Release();

protected:
	PERESOURCE m_resource = nullptr;
};

class ResourceLiteGuard
{
public:
	ResourceLiteGuard( ResourceLite& resource_lite, bool wait );
	~ResourceLiteGuard();

protected:
	ResourceLite& m_resource_lite;
};

}