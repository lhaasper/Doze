#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "no_copy.hpp"
#include "no_move.hpp"

namespace atom::core
{

using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using Minutes = std::chrono::minutes;
using Hours = std::chrono::hours;

template< typename Type >
class Timer : public NoCopy, public NoMove
{
protected:
	using clock = std::chrono::high_resolution_clock;
	using duration = Type;

public:
	Timer( bool begin = true )
		: m_begin()
	{
		if( begin )
		{
			Reset();
		}
	}

public:
	void Reset()
	{
		m_begin = clock::now();
	}

	std::int64_t Elapsed()
	{
		const auto time = std::chrono::duration_cast< duration >( clock::now() - m_begin );
		return static_cast< std::int64_t >( time.count() );
	}

protected:
	clock::time_point m_begin = { };
};

using TimerNanoseconds = Timer< Nanoseconds >;
using TimerMicroseconds = Timer< Microseconds >;
using TimerMilliseconds = Timer< Milliseconds >;
using TimerSeconds = Timer< Seconds >;
using TimerMinutes = Timer< Minutes >;
using TimerHours = Timer< Hours >;

} // namespace atom::core