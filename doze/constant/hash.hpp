#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/string.hpp"
#include "../win32/sdk/unicode_string.hpp"

#include "character.hpp"
#include "string.hpp"

namespace atom::constant
{

constexpr std::uint64_t HashPrime = 1099511628211;
constexpr std::uint64_t HashBasis = 14695981039346656037;

template< typename Type >
constexpr std::uint64_t HashCompute( std::uint64_t hash, const Type* const data, std::size_t size, bool ignore_case )
{
	const auto element = static_cast< std::uint64_t >( ignore_case ? ToLower( data[ 0 ] ) : data[ 0 ] );
	return ( size == 0 ) ? hash : HashCompute( ( hash * HashPrime ) ^ element, data + 1, size - 1, ignore_case );
}

template< typename Type >
constexpr std::uint64_t Hash( const Type* const data, std::size_t size, bool ignore_case )
{
	return HashCompute( HashBasis, data, size, ignore_case );
}

constexpr std::uint64_t Hash( const char* const data, bool ignore_case )
{
	const auto length = GetLength( data );
	return Hash( data, length, ignore_case );
}

constexpr std::uint64_t Hash( const wchar_t* const data, bool ignore_case )
{
	const auto length = GetLength( data );
	return Hash( data, length, ignore_case );
}

constexpr std::uint64_t Hash( const win32::STRING& data, bool ignore_case )
{
	const auto length = GetLength( data );
	return Hash( data.Buffer, length, ignore_case );
}

constexpr std::uint64_t Hash( const win32::UNICODE_STRING& data, bool ignore_case )
{
	const auto length = GetLength( data );
	return Hash( data.Buffer, length, ignore_case );
}

} // namespace atom::constant

#define HASH( Data )																					\
	[ & ]()																											\
	{																														\
		constexpr auto hash = atom::constant::Hash( Data, true );	\
		return hash;																							\
	}()