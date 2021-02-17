#include "string.hpp"

#include <cctype>
#include <cwctype>

#include <string>
#include <algorithm>

namespace atom::core
{

std::string ToString( const std::wstring& data, std::uint32_t code_page /*= CP_ACP*/ )
{
	const auto length = WideCharToMultiByte( code_page, 0, data.c_str(), static_cast< int >( data.length() ), nullptr, 0, nullptr, nullptr );

	if( length )
	{
		std::string output( length, 0 );
		WideCharToMultiByte( code_page, 0, data.c_str(), static_cast< int >( data.length() ), output.data(), length, nullptr, nullptr );
		return std::move( output );
	}

	return { };
}

std::wstring ToWideString( const std::string& data, std::uint32_t code_page /*= CP_ACP*/ )
{
	const auto length = MultiByteToWideChar( code_page, 0, data.c_str(), static_cast< int >( data.length() ), nullptr, 0 );

	if( length )
	{
		std::wstring output( length, 0 );
		MultiByteToWideChar( code_page, 0, data.c_str(), static_cast< int >( data.length() ), output.data(), length );
		return std::move( output );
	}

	return { };
}

std::string ToLower( std::string data )
{
	std::transform( data.begin(), data.end(), data.data(), tolower );
	return data;
}

std::wstring ToLower( std::wstring data )
{
	std::transform( data.begin(), data.end(), data.data(), towlower );
	return data;
}

} // namespace atom::core