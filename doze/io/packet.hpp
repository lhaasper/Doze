#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#define IO_CODE( IoCode )		CTL_CODE( atom::io::FileDevice, IoCode, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

namespace atom::io
{

constexpr std::uint32_t FileDevice = 0x00000022;

constexpr auto SignatureRequest = HASH( "SignatureRequest" );
constexpr auto SignatureResponse = HASH( "SignatureResponse" );

constexpr auto VersionMajor = static_cast< std::uint32_t >( 1 );
constexpr auto VersionMinor = static_cast< std::uint32_t >( 1 );

enum class Operation : std::uint32_t
{
	QueryDriverVersion = IO_CODE( 0x0900 ),		// driver version
	QueryDriverSecure = IO_CODE( 0x0901 ),		// remove MmUnloadedDrivers & PiDDBCacheTable entries
	
	// #TODO:
	QueryDriverQuit = IO_CODE( 0x0902 ),			// remove hooks and free driver image pool

	QueryUserProcess = IO_CODE( 0x0910 ),			// user process information
	QueryUserImage = IO_CODE( 0x0911 ),				// user image information

	UserMemoryCommit = IO_CODE( 0x0920 ),			// commit user memory in process
	UserMemoryFree = IO_CODE( 0x0921 ),				// free user memory in process
	UserMemoryRead = IO_CODE( 0x0922 ),				// read user memory from process
	UserMemoryWrite = IO_CODE( 0x0923 ),			// write user memory to process
	
	// #TODO:
	UserMemoryQuery = IO_CODE( 0x0924 ),			// query user memory information from process
	// UserMemoryProtect = IO_CODE( 0x0925 ),		// set user memory protection in process
};
// sizeof( Operation ) = 0x0004

struct BaseRequest
{
	FORCEINLINE bool IsValid() const
	{
		return ( m_signature == SignatureRequest );
	}

	std::uint64_t m_signature = 0;	// 0x0000
};
// sizeof( BaseRequest ) = 0x0008

struct BaseResponse
{
	FORCEINLINE bool IsValid() const
	{
		return ( m_signature == SignatureResponse );
	}

	FORCEINLINE NTSTATUS GetStatus() const
	{
		return m_status;
	}

	std::uint64_t m_signature = 0;			// 0x0000
	NTSTATUS m_status = STATUS_SUCCESS;	// 0x0008
};
// sizeof( BaseResponse ) = 0x0010

struct QueryDriverVersionRequest : BaseRequest
{ };
// sizeof( QueryDriverVersionRequest ) = 0x0008

struct QueryDriverVersionResponse : BaseResponse
{
	FORCEINLINE std::uint32_t GetMajor() const
	{
		return m_major;
	}

	FORCEINLINE std::uint32_t GetMinor() const
	{
		return m_minor;
	}

	std::uint32_t m_major = 0;	// 0x0010
	std::uint32_t m_minor = 0;	// 0x0014
};
// sizeof( QueryDriverVersionResponse ) = 0x0018

struct QueryDriverSecureRequest : BaseRequest
{
	FORCEINLINE std::uint32_t GetTimeDateStamp() const
	{
		return m_time_date_stamp;
	}

	FORCEINLINE const wchar_t* GetName() const
	{
		return m_name;
	}

	std::uint32_t m_time_date_stamp = 0;	// 0x0008
	wchar_t m_name[ 512 ] = { };					// 0x000C
};
// sizeof( QueryDriverSecureRequest ) = 0x0410

struct QueryDriverQuitRequest : BaseRequest
{ };
// sizeof( QueryDriverQuitRequest ) = 0x0008

struct QueryDriverQuitResponse : BaseResponse
{ };
// sizeof( QueryDriverQuitResponse ) = 0x0010

struct QueryDriverSecureResponse : BaseResponse
{ };
// sizeof( QueryDriverSecureResponse ) = 0x0010

struct QueryUserProcessRequest : BaseRequest
{
	FORCEINLINE std::uint32_t GetProcessId() const
	{
		return m_process_id;
	}

	std::uint32_t m_process_id = 0;	// 0x0008
};
// sizeof( QueryUserProcessRequest ) = 0x0010

struct QueryUserProcessResponse : BaseResponse
{
	FORCEINLINE bool IsWow64() const
	{
		return m_wow64;
	}

	FORCEINLINE std::uintptr_t GetPeb() const
	{
		return m_peb;
	}
	
	FORCEINLINE std::uintptr_t GetBase() const
	{
		return m_base;
	}

	bool m_wow64 = false;				// 0x0010
	std::uintptr_t m_peb = 0;		// 0x0018
	std::uintptr_t m_base = 0;	// 0x0020
};
// sizeof( QueryUserProcessResponse ) = 0x0028

struct QueryUserImageRequest : BaseRequest
{
	FORCEINLINE std::uint32_t GetProcessId() const
	{
		return m_process_id;
	}

	FORCEINLINE const wchar_t* GetName() const
	{
		return m_name;
	}

	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_base = 0;			// 0x0010
	wchar_t m_name[ 512 ] = { };		// 0x0018
};
// sizeof( QueryUserImageRequest ) = 0x0

struct QueryUserImageResponse : BaseResponse
{
	FORCEINLINE std::uintptr_t GetBase() const
	{
		return m_base;
	}
	
	FORCEINLINE std::size_t GetSize() const
	{
		return m_size;
	}

	std::uintptr_t m_base = 0;		// 0x0010
	std::size_t m_size = 0;				// 0x0018
	wchar_t m_name[ 512 ] = { };	// 0x0020
};
// sizeof( QueryUserImageResponse ) = 0x0020

struct UserMemoryCommitRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_address = 0;		// 0x0010
	std::size_t m_size = 0;					// 0x0018
	std::uint32_t m_type = 0;				// 0x0020
	std::uint32_t m_protection = 0;	// 0x0024
	bool m_no_execute = true;				// 0x0028
};
// sizeof( UserMemoryCommitRequest ) = 0x0030

struct UserMemoryCommitResponse : BaseResponse
{
	std::uintptr_t m_address = 0;	// 0x0010
	std::size_t m_size = 0;				// 0x0018
};
// sizeof( UserMemoryCommitResponse ) = 0x0020

struct UserMemoryFreeRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_address = 0;		// 0x0010
	std::size_t m_size = 0;					// 0x0018
	std::uint32_t m_type = 0;				// 0x0020
};
// sizeof( UserMemoryFreeRequest ) = 0x0028

struct UserMemoryFreeResponse : BaseResponse
{ };
// sizeof( UserMemoryFreeResponse ) = 0x0010

struct UserMemoryReadRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_address = 0;		// 0x0010
	std::uintptr_t m_data = 0;			// 0x0018
	std::size_t m_size = 0;					// 0x0020
};
// sizeof( UserMemoryReadRequest ) = 0x0028

struct UserMemoryReadResponse : BaseResponse
{
	std::size_t m_size = 0;	// 0x0010
};
// sizeof( UserMemoryReadResponse ) = 0x0018

struct UserMemoryWriteRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_address = 0;		// 0x0010
	std::uintptr_t m_data = 0;			// 0x0018
	std::size_t m_size = 0;					// 0x0020
};
// sizeof( UserMemoryWriteRequest ) = 0x0028

struct UserMemoryWriteResponse : BaseResponse
{
	std::size_t m_size = 0;	// 0x0010
};
// sizeof( UserMemoryWriteResponse ) = 0x0018

struct UserMemoryQueryRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_address = 0;		// 0x0010
};
// sizeof( UserMemoryQueryRequest ) = 0x0018

struct UserMemoryQueryResponse : BaseResponse
{
	std::uintptr_t m_base_address = 0;			// 0x0010
	std::uintptr_t m_allocation_base = 0;		// 0x0018
	std::uint32_t m_allocation_protect = 0;	// 0x0020
	std::size_t m_region_size = 0;					// 0x0028
	std::uint32_t m_state = 0;							// 0x0030
	std::uint32_t m_protect = 0;						// 0x0034
	std::uint32_t m_type = 0;								// 0x0038
};
// sizeof( UserMemoryReadResponse ) = 0x0040

} // namespace atom::io