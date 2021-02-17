#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../constant/character.hpp"
#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#define IO_CODE( code )		CTL_CODE( atom::io::FileDevice, code, METHOD_OUT_DIRECT, FILE_ANY_ACCESS )

namespace atom::io
{

constexpr auto FileDevice = static_cast< std::uint32_t >( 0x00000022 );

constexpr auto SignatureRequest = HASH( "SignatureRequest" );
constexpr auto SignatureResponse = HASH( "SignatureResponse" );

constexpr auto VersionMajor = static_cast< std::uint32_t >( 1 );
constexpr auto VersionMinor = static_cast< std::uint32_t >( 1 );

enum class Operation : std::uint32_t
{
	QuerySystemVersion = IO_CODE( 0x0900 ),		// driver version

	QuerySecureSystem = IO_CODE( 0x0910 ),		// remove driver from MmUnloadedDrivers & PiDDBCacheTable

	QueryUserProcess = IO_CODE( 0x0920 ),			// user process information
	QueryUserImage = IO_CODE( 0x0921 ),				// user image information

	UserMemoryCommit = IO_CODE( 0x0930 ),			// ZwAllocateVirtualMemory
	UserMemoryFree = IO_CODE( 0x0931 ),				// ZwFreeVirtualMemory
	UserMemoryRead = IO_CODE( 0x0932 ),				// MmCopyVirtualMemory
	UserMemoryWrite = IO_CODE( 0x0933 ),			// MmCopyVirtualMemory
};
// sizeof( Operation ) = 0x0004

enum class Code : std::uint32_t
{
	QueryDriverVersion = IO_CODE( 0x0900 ),
	QueryDriverSecure = IO_CODE( 0x0901 ),
	QueryDriverQuit = IO_CODE( 0x0902 ),

	QueryUserProcess = IO_CODE( 0x0910 ),
	QueryUserImage = IO_CODE( 0x0911 ),

	UserMemoryCommit = IO_CODE( 0x0920 ),
	UserMemoryFree = IO_CODE( 0x0921 ),
	UserMemoryRead = IO_CODE( 0x0922 ),
	UserMemoryWrite = IO_CODE( 0x0923 ),

	UserMemoryQuery = IO_CODE( 0x0924 ),
};

enum CodeType : std::uint32_t
{
	// 
	// driver information
	// 
	CodeQueryVersion = IO_CODE( 0x0900 ),

	// 
	// driver core
	// 
	CodeExecuteSecurity = IO_CODE( 0x0910 ),

	// 
	// process operations
	// 
	CodeQueryProcessData = IO_CODE( 0x0920 ),
	CodeQueryImageData = IO_CODE( 0x0921 ),

	// 
	// virtual memory operations
	// 
	CodeMemoryAllocate = IO_CODE( 0x0930 ),
	CodeMemoryFree = IO_CODE( 0x0931 ),
	CodeMemoryRead = IO_CODE( 0x0932 ),
	CodeMemoryWrite = IO_CODE( 0x0933 ),

	// 
	// physical memory operations
	// 
	CodeMapIoSpace = IO_CODE( 0x0940 ),
	CodeUnmapIoSpace = IO_CODE( 0x0941 ),
};

// 
// use 8-byte structure alignment
// 
#pragma pack( push, 8 )

struct BaseRequest
{
	bool IsValid() const
	{
		return ( m_signature == SignatureRequest );
	}

	std::uint64_t m_signature = SignatureRequest;	// 0x0000
};
// sizeof( BaseRequest ) = 0x0008

struct BaseResponse
{
	bool IsValid() const
	{
		return ( m_signature == SignatureResponse );
	}

	std::uint64_t m_signature = 0;			// 0x0000
	NTSTATUS m_status = STATUS_SUCCESS;	// 0x0008
};
// sizeof( BaseResponse ) = 0x0010

struct QuerySystemVersionRequest : BaseRequest
{ };
// sizeof( QuerySystemVersionRequest ) = 0x0008

struct QuerySystemVersionResponse : BaseResponse
{
	std::uint32_t m_major = 0;	// 0x0010
	std::uint32_t m_minor = 0;	// 0x0014
};
// sizeof( QuerySystemVersionResponse ) = 0x0018

struct QuerySecureSystemRequest : BaseRequest
{
	std::uint32_t m_time_date_stamp = 0;	// 0x0008
	wchar_t m_name[ 512 ] = { };					// 0x000C
};
// sizeof( QuerySecureSystemRequest ) = 0x0410

struct QuerySecureSystemResponse : BaseResponse
{ };
// sizeof( QuerySecureSystemResponse ) = 0x0010

struct QueryDriverQuitRequest : BaseRequest
{ };
// sizeof( QueryDriverQuitRequest ) = 0x0008

struct QueryDriverQuitResponse : BaseResponse
{ };
// sizeof( QueryDriverQuitResponse ) = 0x0010

struct HookDispatchRequest : BaseRequest
{ };

struct HookDispatchResponse : BaseResponse
{ };

struct UnhookDispatchRequest : BaseRequest
{ };

struct UnhookDispatchResponse : BaseResponse
{ };

struct QuitRequest : BaseRequest
{ };

struct QuitResponse : BaseResponse
{ };

struct QueryUserProcessRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;		// 0x0008
};
// sizeof( QueryUserProcessRequest ) = 0x0010

struct QueryUserProcessResponse : BaseResponse
{
	bool m_wow64 = false;				// 0x0010
	std::uintptr_t m_peb = 0;		// 0x0018
	std::uintptr_t m_base = 0;	// 0x0020
};
// sizeof( QueryUserProcessResponse ) = 0x0028

struct QueryUserImageRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;	// 0x0008
	std::uintptr_t m_base = 0;			// 0x0010
	wchar_t m_name[ 512 ] = { };		// 0x0018
};
// sizeof( QueryUserImageRequest ) = 0x0

struct QueryUserImageResponse : BaseResponse
{
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
	std::uint32_t m_process_id = 0;
	std::uintptr_t m_address = 0;
	std::size_t m_size = 0;
	std::uint32_t m_type = 0;
};

struct UserMemoryFreeResponse : BaseResponse
{ };

struct UserMemoryReadRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;
	std::uintptr_t m_address = 0;
	std::uintptr_t m_data = 0;
	std::size_t m_size = 0;
};

struct UserMemoryReadResponse : BaseResponse
{
	std::size_t m_size = 0;
};

struct UserMemoryWriteRequest : BaseRequest
{
	std::uint32_t m_process_id = 0;
	std::uintptr_t m_address = 0;
	std::uintptr_t m_data = 0;
	std::size_t m_size = 0;
};

struct UserMemoryWriteResponse : BaseResponse
{
	std::size_t m_size = 0;
};

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

// 
// restore structure alignment
// 
#pragma pack( pop )

} // namespace atom::io