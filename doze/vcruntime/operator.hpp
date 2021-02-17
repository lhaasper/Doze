#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/pool_type.hpp"

namespace atom::vcruntime
{

void* AllocatePool( win32::POOL_TYPE pool_type, std::size_t number_of_bytes );
void FreePool( void* base_address );

void* AllocateMemory( std::size_t number_of_bytes );
void FreeMemory( void* data );

} // namespace atom::vcruntime