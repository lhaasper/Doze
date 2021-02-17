#pragma once

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <ntstatus.h>
#pragma warning( pop )

#include <optional>
#include <cassert>

namespace atom
{

template< typename Type >
struct invoke_result_t
{
	invoke_result_t() = default;

	invoke_result_t( Type result, NTSTATUS status = STATUS_SUCCESS )
		: m_status( status )
		, m_result( std::move( result ) )
	{
		assert( m_result.has_value() );
	}

	invoke_result_t( NTSTATUS status )
		: m_status( status )
	{
		assert( m_status != STATUS_SUCCESS );
	}

	bool success() const
	{
		return NT_SUCCESS( m_status );
	}

	Type& result()
	{
		return m_result.value();
	}

	const Type& result() const
	{
		return m_result.value();
	}

	Type result( const Type& stock ) const
	{
		return m_result.value_or( stock );
	}

	explicit operator bool() const
	{
		return NT_SUCCESS( m_status );
	}

	explicit operator Type() const
	{
		return m_result.value();
	}

	Type* operator->()
	{
		return &m_result.value();
	}

	Type& operator*()
	{
		return m_result.value();
	}

	NTSTATUS m_status = STATUS_UNSUCCESSFUL;
	std::optional< Type > m_result = std::nullopt;
};

} // namespace atom