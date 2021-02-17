#include "privileges.hpp"

#include "../win32/console.hpp"
#include "../win32/privilege.hpp"

namespace atom
{

constexpr auto cx_debug_privilege_name = L"SeDebugPrivilege";
constexpr auto cx_load_driver_privilege_name = L"SeLoadDriverPrivilege";

bool GrantDebugPrivilege()
{
	win32::Privilege se_debug( cx_debug_privilege_name );

	if( !se_debug.Valid() )
	{
		HORIZON_LOG( "%s: (%ws): not valid!", __FUNCTION__, cx_debug_privilege_name );
		return false;
	}

	if( !se_debug.Set( true ) )
	{
		HORIZON_LOG( "%s: (%ws): failed to grant!", cx_debug_privilege_name );
		return false;
	}

	return true;
}

bool GrantLoadDriverPrivilege()
{
	win32::Privilege se_load_driver( cx_load_driver_privilege_name );

	if( !se_load_driver.Valid() )
	{
		HORIZON_LOG( "%s: (%ws): not valid!", __FUNCTION__, cx_load_driver_privilege_name );
		return false;
	}

	if( !se_load_driver.Set( true ) )
	{
		HORIZON_LOG( "%s: (%ws): failed to grant!", cx_load_driver_privilege_name );
		return false;
	}

	return true;
}

bool GrantRequiredPrivileges()
{
	return ( GrantDebugPrivilege() && GrantLoadDriverPrivilege() );
}

}