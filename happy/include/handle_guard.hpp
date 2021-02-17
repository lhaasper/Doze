#pragma once

#include "auto.hpp"
#include "win32.hpp"

namespace atom
{

template< typename T >
struct non_zero
{
	static bool call( T handle ) noexcept
	{
		return std::intptr_t( handle ) != 0;
	}
};

template< typename T >
struct non_negative
{
	static bool call( T handle ) noexcept
	{
		return std::intptr_t( handle ) > 0;
	}
};

template< template< typename > typename wrapped_t, typename T >
struct with_pseudo_t
{
	static bool call( T handle ) noexcept
	{
		if( wrapped_t< T >::call( handle ) )
		{
			return true;
		}

		auto h = ( HANDLE )( std::uintptr_t )handle;
		return h == GetCurrentProcess() || h == GetCurrentThread();
	}
};

template< template< typename > typename wrapped_t >
struct with_pseudo
{
	template< typename T >
	using type = with_pseudo_t< wrapped_t, T >;
};

template< typename handle_t, auto close, template< typename > typename is_valid = non_negative >
class handle_guard_t
{
public:
	static constexpr handle_t zero_handle = handle_t( 0 );

public:
	explicit handle_guard_t( handle_t handle = zero_handle ) noexcept
		: m_handle( handle )
	{ }

	handle_guard_t( handle_guard_t&& handle_guard ) noexcept
		: m_handle( handle_guard.m_handle )
	{
		handle_guard.m_handle = zero_handle;
	}

	~handle_guard_t()
	{
		if( non_negative< handle_t >::call( m_handle ) )
		{
			close( m_handle );
		}
	}

	handle_guard_t( const handle_guard_t& ) = delete;
	handle_guard_t& operator=( const handle_guard_t& ) = delete;

	handle_guard_t& operator=( handle_guard_t&& handle_guard ) noexcept
	{
		if( std::addressof( handle_guard ) == this )
		{
			return *this;
		}

		reset( handle_guard.m_handle );
		handle_guard.m_handle = zero_handle;

		return *this;
	}

	handle_guard_t& operator=( handle_t handle ) noexcept
	{
		reset( handle );
		return *this;
	}

	void reset( handle_t handle = zero_handle ) noexcept
	{
		if( handle == m_handle )
		{
			return;
		}

		if( non_negative< handle_t >::call( m_handle ) )
		{
			close( m_handle );
		}

		m_handle = handle;
	}

	handle_t release() noexcept
	{
		auto handle = m_handle;
		m_handle = zero_handle;
		return handle;
	}

	handle_t get() const noexcept
	{
		return m_handle;
	}

	bool valid() const noexcept
	{
		return is_valid< handle_t >::call( m_handle );
	}

	operator handle_t() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return valid();
	}

	handle_t* operator&() noexcept
	{
		return &m_handle;
	}

	bool operator==( const handle_guard_t& handle_guard ) const noexcept
	{
		return ( m_handle == handle_guard.m_handle );
	}

	bool operator<( const handle_guard_t& handle_guard ) const noexcept
	{
		return ( m_handle < handle_guard.m_handle );
	}

protected:
	handle_t m_handle;
};

using Handle = handle_guard_t< HANDLE, &CloseHandle >;
using ProcessHandle = handle_guard_t< HANDLE, &CloseHandle, with_pseudo< non_negative >::type >;
using RegistryHandle = handle_guard_t< HKEY, &RegCloseKey >;
using Mapping = handle_guard_t< void*, &UnmapViewOfFile, non_zero >;

} // namespace atom