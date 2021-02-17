#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/singleton.hpp"

#include "../win32/handle_guard.hpp"
#include "../win32/dialog.hpp"
#include "../win32/driver.hpp"
#include "../win32/console.hpp"

#include "packet.hpp"

#define IN_RANGE( x, a, b )	( x >= a && x <= b )
#define GET_BITS( x )		( IN_RANGE( x, '0', '9' ) ? ( x - '0' ) : ( ( x & ( ~0x20 ) ) - 'A' + 0x0A ) )
#define GET_BYTE( x )		( GET_BITS( x[ 0 ] ) << 4 | GET_BITS( x[ 1 ] ) )

#define DEVICE_NULL											L"\\Device\\Null"
#define DEVICE_BEEP											L"\\Device\\Beep"
#define DEVICE_KSECDD										L"\\Device\\KsecDD"
#define DEVICE_LANMAN_DATAGRAM_RECEIVER	L"\\Device\\LanmanDatagramReceiver"

namespace atom::io
{

struct UserProcessData
{
	bool m_wow64 = false;
	std::uintptr_t m_peb = 0;
	std::uintptr_t m_base = 0;
};

struct UserImageData
{
	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;
	wchar_t m_name[ 512 ] = { };
};

struct ProcessData
{
	std::uint32_t m_process_id = 0;
	bool m_wow64 = false;
	std::uintptr_t m_peb = 0;
};

struct ImageData
{
	std::uint32_t m_process_id = 0;
	std::wstring m_name = { };
	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;
};

struct MemoryBlock
{
	static MemoryBlock Allocate( std::size_t size, std::uint32_t protection = PAGE_READWRITE );
	void Free();

	void Reset();

	bool IsValid() const;

	template< typename Type >
	void Read( Type* data, std::size_t size = sizeof( Type ), std::ptrdiff_t offset = 0 );

	template< typename Type >
	void Write( const Type* const data, std::size_t size = sizeof( Type ), std::ptrdiff_t offset = 0 );

	std::uint32_t m_process_id = 0;
	std::uintptr_t m_address = 0;
	std::size_t m_size = 0;
	std::uint32_t m_protection = PAGE_NOACCESS;
};

struct UserMemoryData;

class DriverControl : public core::Singleton< DriverControl >
{
public:
	~DriverControl();

public:
	bool Create();
	void Destroy();
	
	bool IsValid() const;

public:
	bool Attach( const wchar_t* const class_name );
	void Detach();

public:
	FORCEINLINE HWND GetWindow() const
	{
		return m_window;
	}

	FORCEINLINE std::uint32_t GetThreadId() const
	{
		return m_thread_id;
	}

	FORCEINLINE std::uint32_t GetProcessId() const
	{
		return m_process_id;
	}

	FORCEINLINE void SetWindow( HWND window )
	{
		m_window = window;
	}

	FORCEINLINE void SetThreadId( std::uint32_t thread_id )
	{
		m_thread_id = thread_id;
	}

	FORCEINLINE void SetProcessId( std::uint32_t process_id )
	{
		m_process_id = process_id;
	}

public:
	bool QueryDriverVersion( std::uint32_t* major, std::uint32_t* minor );
	bool QueryDriverSecure( const wchar_t* const name, std::uint32_t time_date_stamp );
	bool QueryDriverQuit();

	bool QueryUserProcess( UserProcessData* user_process );
	bool QueryUserImage( const wchar_t* const name, UserImageData* user_image );
	bool QueryUserImage( std::uintptr_t base, UserImageData* user_image );

	std::uintptr_t UserMemoryCommit( std::uintptr_t address, std::size_t size, std::uint32_t type, std::uint32_t protection, bool no_execute, std::size_t* output );
	bool UserMemoryFree( std::uintptr_t address, std::size_t size, std::uint32_t type );

	bool UserMemoryRead( std::uintptr_t address, void* data, std::size_t size, std::size_t* size_result );
	bool UserMemoryWrite( std::uintptr_t address, const void* const data, std::size_t size, std::size_t* size_result );

	bool UserMemoryQuery( std::uintptr_t address, MEMORY_BASIC_INFORMATION* information );

public:
	std::uintptr_t GetPeb();
	std::uintptr_t GetImage( const wchar_t* const name );
	std::wstring GetImageName( std::uintptr_t address );

	std::uintptr_t CommitMemory( std::size_t size, std::uint32_t type, std::uint32_t protection, bool no_execute = false, std::size_t* output = nullptr );
	bool FreeMemory( std::uintptr_t address, std::size_t size, std::uint32_t type );

public:
	UserMemoryData Commit( std::size_t size, std::uint32_t protection = PAGE_READWRITE );
	bool Free( UserMemoryData& user_memory );

public:
	template< typename Type >
	bool ReadMemory( std::uintptr_t address, Type* data, std::size_t size = sizeof( Type ), std::size_t* size_result = nullptr )
	{
		return UserMemoryRead( address, static_cast< void* >( data ), size, size_result );
	}

	template< typename Type >
	bool WriteMemory( std::uintptr_t address, const Type* const data, std::size_t size = sizeof( Type ), std::size_t* size_result = nullptr )
	{
		return UserMemoryWrite( address, static_cast< const void* >( data ), size, size_result );
	}

	template< typename Type >
	Type Read( std::uintptr_t address )
	{
		Type data = { };
		std::size_t result = 0;

		if( !ReadMemory( address, &data, sizeof( data ), &result ) )
		{
			return { };
		}

		if( result != sizeof( data ) )
		{
			TRACE( "%s: ReadMemory( ... ) size mismatch! '0x%016llX' != '0x%016llX'", __FUNCTION__, result, sizeof( data ) );
		}

		return data;
	}

	template< typename Type >
	bool Write( std::uintptr_t address, const Type& data )
	{
		std::size_t result = 0;

		if( !WriteMemory( address, &data, sizeof( data ), &result ) )
		{
			return false;
		}

		if( result != sizeof( data ) )
		{
			TRACE( "%s: WriteMemory( ... ) size mismatch! '0x%016llX' != '0x%016llX'", __FUNCTION__, result, sizeof( data ) );
		}

		return true;
	}

protected:
	template< typename CodeType, typename RequestType, typename ResponseType >
	bool DeviceControl( const CodeType io_code, const RequestType* const request, ResponseType* response )
	{
		IO_STATUS_BLOCK io_status_block = { };

		const auto status = NtDeviceIoControlFile( m_device, nullptr, nullptr, nullptr,
																							 &io_status_block,
																							 static_cast< ULONG >( io_code ),
																							 PVOID( request ),
																							 static_cast< ULONG >( sizeof( RequestType ) ),
																							 PVOID( response ),
																							 static_cast< ULONG >( sizeof( ResponseType ) ) );

		if( !NT_SUCCESS( status ) )
		{
			TRACE( "%s: NtDeviceIoControlFile( ... ) error! (0x%08X)", __FUNCTION__, status );
			return false;
		}

		return true;
	}

protected:
	bool CreateDevice();
	void DestroyDevice();

protected:
	win32::NtHandle m_device = { };
	std::wstring m_device_name = { };

	HWND m_window = nullptr;

	std::uint32_t m_thread_id = 0;
	std::uint32_t m_process_id = 0;
};

template< typename Type >
void MemoryBlock::Read( Type* data, std::size_t size /*= sizeof( Type )*/, std::ptrdiff_t offset /*= 0*/ )
{
	auto& driver = DriverControl::Instance();

	if( !driver.ReadMemory( m_address + offset, data, size ) )
	{
		throw std::runtime_error( __FUNCTION__ ": ReadMemory( ... ) error! (" + std::to_string( GetLastError() ) + ")" );
	}
}

template< typename Type >
void MemoryBlock::Write( const Type* const data, std::size_t size /*= sizeof( Type )*/, std::ptrdiff_t offset /*= 0*/ )
{
	auto& driver = DriverControl::Instance();

	if( !driver.WriteMemory( m_address + offset, data, size ) )
	{
		throw std::runtime_error( __FUNCTION__ ": WriteMemory( ... ) error! (" + std::to_string( GetLastError() ) + ")" );
	}
}

struct UserMemoryData
{
	FORCEINLINE bool IsValid() const
	{
		return ( m_address && m_size && m_protection != PAGE_NOACCESS );
	}

	template< typename Type >
	FORCEINLINE bool Read( Type* const data, std::size_t size = sizeof( Type ), std::intptr_t offset = 0 )
	{
		auto& doze = DriverControl::Instance();

		if( IsValid() )
		{
			std::size_t number_of_bytes_read = 0;

			if( doze.ReadMemory( m_address + offset, data, size, &number_of_bytes_read ) )
			{
				return ( number_of_bytes_read == size );
			}
		}

		return false;
	}

	template< typename Type >
	FORCEINLINE bool Write( const Type* const data, std::size_t size = sizeof( Type ), std::intptr_t offset = 0 )
	{
		auto& doze = DriverControl::Instance();

		if( IsValid() )
		{
			std::size_t number_of_bytes_written = 0;

			if( doze.WriteMemory( m_address + offset, data, size, &number_of_bytes_written ) )
			{
				return ( number_of_bytes_written == size );
			}
		}

		return false;
	}

	template< typename Type >
	FORCEINLINE Type Read( std::intptr_t offset = 0 )
	{
		Type data = { };
		Read( &data, sizeof( data ), offset );
		return data;
	}

	std::uintptr_t m_address = 0;
	std::size_t m_size = 0;
	std::uint32_t m_protection = PAGE_NOACCESS;
};

} // namespace atom::io