#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::core
{

class NoCopy
{
protected:
	NoCopy() = default;
	NoCopy( const NoCopy& ) = delete;

protected:
	NoCopy& operator=( const NoCopy& ) = delete;
};

}