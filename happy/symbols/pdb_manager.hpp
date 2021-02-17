#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../dia/dia2.h"

#include <objbase.h>
#include <atlcomcli.h>

#include <string>
#include <sstream>
#include <unordered_map>

namespace atom
{

class PDBManager
{
public:
	PDBManager();
	~PDBManager();

public:
	HRESULT Initialize( const std::wstring& file, std::uintptr_t base = 0 );
	HRESULT GetSymbol( const std::wstring& name, std::uintptr_t& result );

protected:
	HRESULT CreateDiaDataSource();
	HRESULT PopulateSymbols();

protected:
	CComPtr< IDiaDataSource > m_source = { };
	CComPtr< IDiaSession > m_session = { };
	CComPtr< IDiaSymbol > m_scope = { };

	std::uintptr_t m_base = 0;
	std::wstring m_directory = { };
	std::unordered_map< std::wstring, std::uint32_t > m_symbols = { };
};

} // namespace atom