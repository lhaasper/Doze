#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/no_copy.hpp"

#include "handle_guard.hpp"

namespace atom::win32
{

class Driver : public core::NoCopy
{
public:
	Driver();
	Driver( const std::wstring& name );

public:
	~Driver();

public:
	bool Open();
	void Close();

public:
	bool IsOpen() const;

	const std::wstring& GetName() const;
	
	bool DeviceControl( std::uint32_t io_control_code,
											void* input_data,
											std::size_t input_data_size,
											void* output_data,
											std::size_t output_data_size );

public:
	template< typename IoCodeType, typename InputDataType, typename OutputDataType >
	FORCEINLINE bool DeviceControl( IoCodeType io_code,
																	InputDataType* input_data,
																	OutputDataType* output_data )
	{
		return DeviceControl( static_cast< std::uint32_t >( io_code ),
													static_cast< void* >( input_data ),
													sizeof( InputDataType ),
													static_cast< void* >( output_data ),
													sizeof( OutputDataType ) );
	}

protected:
	std::wstring m_name = { };
	NtHandle m_device = { };
};

} // namespace atom::win32