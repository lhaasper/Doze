#include "driver_control.hpp"

#include "../win32/debug_trace.hpp"
#include "../win32/console.hpp"

namespace atom::io
{

// 
// [ DriverControl ] implementation
// 
DriverControl::~DriverControl()
{
	Destroy();
}

bool DriverControl::Create()
{
	if( m_device.IsValid() )
	{
		TRACE( "%s: Driver is already loaded!", __FUNCTION__ );
		return true;
	}

	Destroy();

	// 
	// set device object name to communicate with
	// 
	m_device_name.assign( DEVICE_LANMAN_DATAGRAM_RECEIVER );

	// 
	// open device object
	// 
	return CreateDevice();
}

void DriverControl::Destroy()
{
	// 
	// detach from current process
	// 
	Detach();

	// 
	// close device driver io
	// 
	DestroyDevice();
}

bool DriverControl::IsValid() const
{
	return m_device.IsValid();
}

bool DriverControl::Attach( const wchar_t* const class_name )
{
	if( m_thread_id && m_process_id )
	{
		TRACE( "%s: Driver already attached to process '0x%08X'!", __FUNCTION__, m_process_id );
		return true;
	}

	if( !class_name )
	{
		m_window = GetDesktopWindow();
		m_thread_id = GetCurrentThreadId();
		m_process_id = GetCurrentProcessId();
		return true;
	}

	m_window = FindWindowW( class_name, nullptr );

	if( !m_window )
	{
		TRACE( "%s: FindWindowW( '%S' ) error! (0x%08X)", __FUNCTION__, class_name, GetLastError() );
		return false;
	}

	auto process_id = static_cast< DWORD >( 0 );
	auto thread_id = GetWindowThreadProcessId( m_window, &process_id );

	if( !thread_id || !process_id )
	{
		TRACE( "%s: GetWindowThreadProcessId( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	m_thread_id = static_cast< std::uint32_t >( thread_id );
	m_process_id = static_cast< std::uint32_t >( process_id );

	return true;
}

void DriverControl::Detach()
{
	m_window = nullptr;
	m_thread_id = 0;
	m_process_id = 0;
}

bool DriverControl::QueryDriverVersion( std::uint32_t* major, std::uint32_t* minor )
{
	QuerySystemVersionRequest request = { };
	QuerySystemVersionResponse response = { };

	const auto io_code = Code::QueryDriverVersion;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( major )
	{
		// 
		// set major value
		// 
		*major = response.m_major;
	}

	if( minor )
	{
		// 
		// set minor value
		// 
		*minor = response.m_minor;
	}

	return true;
}

bool DriverControl::QueryDriverSecure( const wchar_t* const name, std::uint32_t time_date_stamp )
{
	QuerySecureSystemRequest request = { };
	QuerySecureSystemResponse response = { };

	wcscpy_s( request.m_name, name );
	request.m_time_date_stamp = time_date_stamp;

	const auto io_code = Code::QueryDriverSecure;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	return true;
}

bool DriverControl::QueryDriverQuit()
{
	QueryDriverQuitRequest request = { };
	QueryDriverQuitResponse response = { };

	const auto io_code = Code::QueryDriverQuit;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	return true;
}

bool DriverControl::QueryUserProcess( UserProcessData* user_process )
{
	QueryUserProcessRequest request = { };
	QueryUserProcessResponse response = { };

	request.m_process_id = m_process_id;
	
	if( user_process )
	{
		// 
		// clear process data
		// 
		std::memset( user_process, 0, sizeof( UserProcessData ) );
	}

	const auto io_code = Code::QueryUserProcess;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( user_process )
	{
		user_process->m_wow64 = response.m_wow64;
		user_process->m_peb = response.m_peb;
		user_process->m_base = response.m_base;
	}

	return true;
}

bool DriverControl::QueryUserImage( const wchar_t* const name, UserImageData* user_image )
{
	QueryUserImageRequest request = { };
	QueryUserImageResponse response = { };

	request.m_process_id = m_process_id;
	request.m_base = 0;
	wcscpy_s( request.m_name, name );
	
	if( user_image )
	{
		// 
		// clear image data
		// 
		std::memset( user_image, 0, sizeof( UserImageData ) );
		wcscpy_s( user_image->m_name, name );
	}

	const auto io_code = Code::QueryUserImage;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( user_image )
	{
		// image_data->m_process_id = m_process_id;
		// image_data->m_name = name;
		user_image->m_base = response.m_base;
		user_image->m_size = static_cast< std::size_t >( response.m_size );
	}

	return true;
}

bool DriverControl::QueryUserImage( std::uintptr_t base, UserImageData* user_image )
{
	QueryUserImageRequest request = { };
	QueryUserImageResponse response = { };

	request.m_process_id = m_process_id;
	request.m_base = base;
	// wcscpy_s( request.m_name, name );

	if( user_image )
	{
		// 
		// clear image data
		// 
		std::memset( user_image, 0, sizeof( UserImageData ) );
	}

	const auto io_code = Code::QueryUserImage;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( user_image )
	{
		// image_data->m_process_id = m_process_id;
		// image_data->m_name = name;
		user_image->m_base = response.m_base;
		user_image->m_size = static_cast< std::size_t >( response.m_size );
		wcscpy_s( user_image->m_name, response.m_name );
		// std::memcpy( user_image->m_name, response.m_name, sizeof( response.m_name ) );
	}

	return true;
}

std::uintptr_t DriverControl::UserMemoryCommit( std::uintptr_t address, std::size_t size, std::uint32_t type, std::uint32_t protection, bool no_execute, std::size_t* size_result )
{
	UserMemoryCommitRequest request = { };
	UserMemoryCommitResponse response = { };

	request.m_process_id = m_process_id;
	request.m_address = address;
	request.m_size = size;
	request.m_type = type;
	request.m_protection = protection;
	request.m_no_execute = no_execute;

	const auto io_code = Code::UserMemoryCommit;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return 0;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return 0;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return 0;
	}

	if( size_result )
	{
		// 
		// set number of bytes allocated
		// 
		*size_result = response.m_size;
	}

	if( size != response.m_size )
	{
		TRACE( "%s: Allocation size mismatch: '0x%016llX' != '0x%016llX'!", __FUNCTION__, size, response.m_size );
	}

	return response.m_address;
}

bool DriverControl::UserMemoryFree( std::uintptr_t address, std::size_t size, std::uint32_t type )
{
	UserMemoryFreeRequest request = { };
	UserMemoryFreeResponse response = { };

	request.m_process_id = m_process_id;
	request.m_address = address;
	request.m_size = size;
	request.m_type = type;

	const auto io_code = Code::UserMemoryFree;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	return true;
}

bool DriverControl::UserMemoryRead( std::uintptr_t address, void* data, std::size_t size, std::size_t* size_result )
{
	UserMemoryReadRequest request = { };
	UserMemoryReadResponse response = { };

	request.m_process_id = m_process_id;
	request.m_address = address;
	request.m_data = reinterpret_cast< std::uintptr_t >( data );
	request.m_size = size;

	const auto io_code = Code::UserMemoryRead;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( size_result )
	{
		// 
		// set number of bytes copied
		// 
		*size_result = response.m_size;
	}

	return true;
}

bool DriverControl::UserMemoryWrite( std::uintptr_t address, const void* const data, std::size_t size, std::size_t* size_result )
{
	UserMemoryWriteRequest request = { };
	UserMemoryWriteResponse response = { };

	request.m_process_id = m_process_id;
	request.m_address = address;
	request.m_data = reinterpret_cast< std::uintptr_t >( data );
	request.m_size = size;

	const auto io_code = Code::UserMemoryWrite;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( size_result )
	{
		// 
		// set number of bytes copied
		// 
		*size_result = response.m_size;
	}

	return true;
}

bool DriverControl::UserMemoryQuery( std::uintptr_t address, MEMORY_BASIC_INFORMATION* information )
{
	UserMemoryQueryRequest request = { };
	UserMemoryQueryResponse response = { };

	request.m_process_id = m_process_id;
	request.m_address = address;

	const auto io_code = Code::UserMemoryQuery;

	if( !DeviceControl( io_code, &request, &response ) )
	{
		TRACE( "%s: DeviceIoControl( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	if( !response.IsValid() )
	{
		TRACE( "%s: Driver response is not valid!", __FUNCTION__ );
		return false;
	}

	if( !NT_SUCCESS( response.m_status ) )
	{
		TRACE( "%s: Driver control @ '0x%08X' error! (0x%08X)", __FUNCTION__, io_code, response.m_status );
		return false;
	}

	if( information )
	{
		std::memcpy( information, &response.m_base_address, sizeof( MEMORY_BASIC_INFORMATION ) );
		// 
		// set number of bytes copied
		// 
		// *size_result = response.m_size;
	}

	return true;
}

std::uintptr_t DriverControl::GetPeb()
{
	UserProcessData user_process = { };

	if( !QueryUserProcess( &user_process ) )
	{
		TRACE( "%s: QueryUserProcess( ... ) error!", __FUNCTION__ );
		return 0;
	}

	return user_process.m_peb;
}

std::uintptr_t DriverControl::GetImage( const wchar_t* const name )
{
	UserImageData user_image = { };

	if( !QueryUserImage( name, &user_image ) )
	{
		TRACE( "%s: QueryUserImage( '%S' ) error!", __FUNCTION__, name );
		return 0;
	}

	return user_image.m_base;
}

std::wstring DriverControl::GetImageName( std::uintptr_t address )
{
	UserImageData user_image = { };

	if( !QueryUserImage( address, &user_image ) )
	{
		TRACE( "%s: QueryUserImage( '0x%016llX' ) error!", __FUNCTION__, address );
		return { };
	}

	return { user_image.m_name };
}

std::uintptr_t DriverControl::CommitMemory( std::size_t size, std::uint32_t type, std::uint32_t protection, bool no_execute /*= false*/, std::size_t* size_result /*= nullptr*/ )
{
	return UserMemoryCommit( 0, size, type, protection, no_execute, size_result );
}

bool DriverControl::FreeMemory( std::uintptr_t address, std::size_t size, std::uint32_t type )
{
	return UserMemoryFree( address, size, type );
}

UserMemoryData DriverControl::Commit( std::size_t size, std::uint32_t protection /*= PAGE_READWRITE*/ )
{
	UserMemoryData user_memory = { };

	user_memory.m_address = CommitMemory( size, MEM_COMMIT | MEM_RESERVE, protection, false, &user_memory.m_size );
	user_memory.m_protection = protection;

	return std::move( user_memory );
}

bool DriverControl::Free( UserMemoryData& user_memory )
{
	if( user_memory.IsValid() )
	{
		return FreeMemory( user_memory.m_address, 0, MEM_RELEASE );
	}

	return false;
}

bool DriverControl::CreateDevice()
{
	UNICODE_STRING device_name = { };
	RtlInitUnicodeString( &device_name, m_device_name.c_str() );

	OBJECT_ATTRIBUTES object_attributes =
	{
		sizeof( OBJECT_ATTRIBUTES ),
		nullptr, &device_name,
	};

	IO_STATUS_BLOCK io_status_block = { };

	const auto status = NtOpenFile( &m_device,
																	FILE_READ_DATA,
																	&object_attributes,
																	&io_status_block,
																	FILE_SHARE_READ,
																	FILE_OPEN_IF );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: NtOpenFile( ... ) error! (0x%08X)", __FUNCTION__, status );
		return false;
	}

	return m_device.IsValid();
}

void DriverControl::DestroyDevice()
{
	m_device.Reset();
	m_device_name.clear();
}

// 
// [ MemoryBlock ] implementation
// 
MemoryBlock MemoryBlock::Allocate( std::size_t size, std::uint32_t protection /*= PAGE_READWRITE*/ )
{
	auto& driver = DriverControl::Instance();

	MemoryBlock block = { };

	block.m_process_id = driver.GetProcessId();
	block.m_address = driver.CommitMemory( size, MEM_COMMIT | MEM_RESERVE, protection, &block.m_size );
	block.m_protection = protection;

	if( !block.IsValid() )
	{
		throw std::runtime_error( __FUNCTION__ ": AllocateMemory( ... ) error!" );
	}

	return std::move( block );
}

void MemoryBlock::Free()
{
	auto& driver = DriverControl::Instance();

	if( IsValid() )
	{
		if( !driver.FreeMemory( m_address, 0, MEM_RELEASE ) )
		{
			throw std::runtime_error( __FUNCTION__ ": FreeMemory( ... ) error!" );
		}
	}
}

void MemoryBlock::Reset()
{
	m_address = 0;
	m_size = 0;
	m_protection = PAGE_NOACCESS;
}

bool MemoryBlock::IsValid() const
{
	return ( m_process_id != 0 &&
					 m_address != 0 &&
					 m_size != 0 );
}

} // namespace atom::io