#include "driver.hpp"
#include "console.hpp"
#include "native.hpp"

#include "../core/string.hpp"

#include "../memory/operation.hpp"

namespace atom::win32
{

Driver::Driver()
	: m_name()
	, m_device()
{ }

Driver::Driver( const std::wstring& name )
	: m_name( name )
	, m_device()
{
	if( !Open() )
	{
		throw std::runtime_error( "Couldn't open handle to device driver '" + core::ToString( name ) + "'!" );
	}
}

Driver::~Driver()
{
	Close();
}

bool Driver::Open()
{
	if( IsOpen() )
	{
		TRACE( "%s: Handle to device driver is already open: '0x%016llX'", __FUNCTION__, memory::ToAddress( m_device.Get() ) );
		return true;
	}

	OBJECT_ATTRIBUTES object_attributes = { };
	InitializeObjectAttributes( &object_attributes, m_name );

	IO_STATUS_BLOCK io_status_block = { };

	const auto status = NtOpenFile( &m_device,
																	FILE_READ_DATA,
																	&object_attributes,
																	&io_status_block,
																	FILE_SHARE_READ,
																	FILE_OPEN_IF );

	if( NT_SUCCESS( status ) )
	{
		TRACE( "%s: Handle to device driver: '0x%016llX'", __FUNCTION__, memory::ToAddress( m_device.Get() ) );
	}
	else
	{
		TRACE( "%s: NtOpenFile( ... ) error! (0x%08X)", __FUNCTION__, status );
	}

	return m_device.IsValid();
}

void Driver::Close()
{
	m_device.Reset();
}

bool Driver::IsOpen() const
{
	return m_device.IsValid();
}

const std::wstring& Driver::GetName() const
{
	return m_name;
}

bool Driver::DeviceControl( std::uint32_t io_control_code,
														void* input_data,
														std::size_t input_data_size,
														void* output_data,
														std::size_t output_data_size )
{
	if( m_device.IsValid() )
	{
		IO_STATUS_BLOCK io_status_block = { };

		const auto status = NtDeviceIoControlFile( m_device,
																							 nullptr,
																							 nullptr,
																							 nullptr,
																							 &io_status_block,
																							 static_cast< ULONG >( io_control_code ),
																							 input_data,
																							 static_cast< ULONG >( input_data_size ),
																							 output_data,
																							 static_cast< ULONG >( output_data_size ) );

		if( NT_SUCCESS( status ) )
		{
			TRACE( "%s: Sent '0x%08X' control code to device driver!", __FUNCTION__, io_control_code );
			return true;
		}
		else
		{
			TRACE( "%s: NtDeviceIoControlFile( ... ) error! (0x%08X)", __FUNCTION__, status );
			return false;
		}
	}
	else
	{
		TRACE( "%s: Handle to device driver is not open!", __FUNCTION__ );
		return false;
	}
}

} // namespace atom::win32