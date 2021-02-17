#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../symbols/symbol_data.hpp"

namespace atom
{

using SystemSymbolData = atom::SymbolData;

struct SystemMapData
{
	std::uint64_t m_signature = 0;		// 0x0000
	std::uintptr_t m_base = 0;				// 0x0008
	std::size_t m_size = 0;						// 0x0010
	SystemSymbolData m_symbols = { };	// 0x0018
};

} // namespace atom

namespace atom
{

struct DriverMapContext
{
	std::uint64_t m_magic = 0;

	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;

	SymbolData m_symbols = { };
};

} // namespace atom