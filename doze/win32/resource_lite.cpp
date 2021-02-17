#include "resource_lite.hpp"

#include "../memory/macro.hpp"
#include "../memory/operation.hpp"

namespace atom::win32
{

// 
// [ ResourceLite ] implementation
// 
ResourceLite::ResourceLite( PERESOURCE resource )
	: m_resource( resource )
{ }

bool ResourceLite::IsValid() const
{
	return memory::IsAddressValid( m_resource );
}

bool ResourceLite::AcquireShared( bool wait )
{
	return ( ExAcquireResourceSharedLite( m_resource, ( wait ? TRUE : FALSE ) ) != FALSE );
}

bool ResourceLite::AcquireExclusive( bool wait )
{
	return ( ExAcquireResourceSharedLite( m_resource, ( wait ? TRUE : FALSE ) ) != FALSE );
}

void ResourceLite::Release()
{
	ExReleaseResourceLite( m_resource );
}

// 
// [ ResourceLiteGuard ] implementation
// 
ResourceLiteGuard::ResourceLiteGuard( ResourceLite& resource_lite, bool wait )
	: m_resource_lite( resource_lite )
{
	if( m_resource_lite.IsValid() )
	{
		m_resource_lite.AcquireShared( wait );
	}
}

ResourceLiteGuard::~ResourceLiteGuard()
{
	if( m_resource_lite.IsValid() )
	{
		m_resource_lite.Release();
	}
}

}