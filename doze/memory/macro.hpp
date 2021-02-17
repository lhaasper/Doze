#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#define FIELD_PAD( Size	)																			\
	protected:																									\
		std::uint8_t JOIN( Reserved, __COUNTER__ )[ Size ] = { };	\
	public:

namespace atom::memory
{

} // namespace atom::memory