#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::core
{

std::string ToString( const std::wstring& data, std::uint32_t code_page = CP_ACP );
std::wstring ToWideString( const std::string& data, std::uint32_t code_page = CP_ACP );

std::string ToLower( std::string data );
std::wstring ToLower( std::wstring data );

} // namespace atom::core