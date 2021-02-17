#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

// #include "base.hpp"
// #include "sdk/portable_executable.hpp"

#include "sdk/image_dos_header.hpp"
#include "sdk/image_nt_headers.hpp"

#include "sdk/image_section_header.hpp"
#include "sdk/image_export_directory.hpp"

#include "sdk/peb.hpp"
#include "sdk/ldr_data_table_entry.hpp"
#include "sdk/kldr_data_table_entry.hpp"

namespace atom::win32
{

const IMAGE_DOS_HEADER* GetImageDosHeader( std::uintptr_t image );
const IMAGE_NT_HEADERS* GetImageNtHeaders( std::uintptr_t image );

const IMAGE_DATA_DIRECTORY* GetImageDataDirectory( std::uintptr_t image, std::uint16_t directory );

const IMAGE_SECTION_HEADER* GetImageSection( std::uintptr_t image, const char* const name );
const IMAGE_SECTION_HEADER* GetImageSection( std::uintptr_t image, std::uint64_t name_hash );

const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, const wchar_t* const name );
const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, std::uint64_t name_hash );
const LDR_DATA_TABLE_ENTRY* GetLdrDataTableEntry( const LIST_ENTRY* list_head, std::uintptr_t address );

const LIST_ENTRY* GetPsLoadedModuleList();
const ERESOURCE* GetPsLoadedModuleResource();

KLDR_DATA_TABLE_ENTRY* GetSystemLdrDataTableEntry( LIST_ENTRY* list_head, const wchar_t* const name );
KLDR_DATA_TABLE_ENTRY* GetSystemLdrDataTableEntry( LIST_ENTRY* list_head, std::uint64_t hash );

std::uintptr_t GetUserImage( const PEPROCESS process, const wchar_t* const name );
std::uintptr_t GetUserImage( const PEPROCESS process, std::uint64_t name_hash );

bool GetUserImageName( const PEPROCESS process, std::uintptr_t address, wchar_t* name );

std::uintptr_t GetSystemImage( const wchar_t* const name );
std::uintptr_t GetSystemImage( std::uint64_t name_hash );

std::uintptr_t GetKernelImage( const wchar_t* const name );
std::uintptr_t GetKernelImage( std::uint64_t name_hash );

std::uintptr_t GetImageExport( std::uintptr_t image, const char* const name );
std::uintptr_t GetImageExport( std::uintptr_t image, std::uint64_t name_hash );

}