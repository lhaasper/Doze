#pragma once

#include "symbol_data.hpp"

#include <string>

namespace atom
{

class SymbolLoader
{
public:
	SymbolLoader();

public:
	NTSTATUS Load( SymbolData& symbols );
	
	NTSTATUS LoadFromSymbols( SymbolData& symbols );

protected:
	bool m_x86_os = false;
	bool m_wow64_process = false;
};


} // namespace atom