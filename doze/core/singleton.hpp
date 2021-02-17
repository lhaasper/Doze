#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "no_copy.hpp"
#include "no_move.hpp"

namespace atom::core
{

template< class Type >
class Singleton : public NoCopy, public NoMove
{
public:
	static Type& Instance()
	{
		static Type instance = { };
		return instance;
	}
};

}