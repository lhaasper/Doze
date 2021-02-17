#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/singleton.hpp"
#include "../io/driver_control.hpp"

#include <array>

namespace atom
{

/*
using namespace asmjit;

class MMap : public core::Singleton< MMap >
{
protected:
	static constexpr std::array< x86::Gp, 4 > cx_reg_pack =
	{
		x86::rcx, x86::rdx, x86::r8, x86::r9,
	};

public:
	MMap();

public:
	~MMap();
	
public:
	bool Create( const std::wstring& class_name );
	void Destroy();

public:
	bool MapImage( const std::wstring& name );
	bool MapImage( const std::uint8_t* const data, std::size_t size );

protected:
	bool CopyHeaders( const std::uint8_t* const data );
	bool CopySections( const std::uint8_t* const data );
	bool ProcessRelocations( const std::uint8_t* const data );

	void GenerateBegin();
	void GenerateArg( std::size_t index, std::uint64_t arg );
	void GenerateCall( std::uintptr_t procedure, const std::vector< std::uint64_t >& arg_pack );
	void GenerateEnd();

protected:
	JitRuntime m_runtime;
	CodeHolder m_code;
	x86::Assembler m_assembler;

	std::wstring m_class_name = { };
	HWND m_window = nullptr;
	std::uint32_t m_thread_id = 0;
	std::uint32_t m_process_id = 0;

	std::unique_ptr< std::uint8_t[] > m_image_data = { };
	std::uintptr_t m_image_base = 0;
	std::size_t m_image_size = 0;

	IMAGE_DOS_HEADER* m_image_dos_header = nullptr;
	IMAGE_NT_HEADERS* m_image_nt_headers = nullptr;
};*/

std::uint32_t GetProcessIdWithMaxThreadCount( const std::wstring& name );

std::uint64_t ExecuteProcedure( std::uint32_t process_id, std::uint32_t thread_id, std::uintptr_t procedure, std::uint64_t rcx, std::uint64_t rdx, std::uint64_t r8 );

// bool MapImage( io::Control& control, const std::uint8_t* library_data, std::size_t library_size );
bool MapUserImage( const std::uint8_t* library_data, std::size_t library_size );

} // namespace atom