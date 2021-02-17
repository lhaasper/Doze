#include "system_map_data.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

namespace atom
{

bool SystemMapData::IsValid() const
{
	return ( m_signature == HASH( "SystemMapData" ) );
}

} // namespace atom

atom::SystemMapData g_map_data = { };