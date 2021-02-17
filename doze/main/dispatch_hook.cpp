#include "dispatch_hook.hpp"

#include "../constant/hash.hpp"

#include "../io/control.hpp"
#include "../io/packet.hpp"

#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../win32/image.hpp"
#include "../win32/page.hpp"
#include "../win32/process.hpp"
#include "../win32/trace.hpp"

namespace atom
{

DRIVER_OBJECT* g_driver_object = nullptr;
DRIVER_DISPATCH* g_driver_dispatch = nullptr;

const win32::IMAGE_SECTION_HEADER* g_driver_data_section = nullptr;

std::uintptr_t g_driver_data_hook = 0;
std::size_t g_driver_data_hook_size = 0;

std::uint8_t g_driver_dispatch_hook[ 48 ] =
{
	0x48, 0x89, 0x54, 0x24, 0x10,																// mov [ rsp + 10 ], rdx
	0x48, 0x89, 0x4C, 0x24, 0x08,																// mov [ rsp + 08 ], rcx
	0x48, 0x83, 0xEC, 0x38,																			// sub rsp, 38
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov rax, 0000000000000000
	0x48, 0x89, 0x44, 0x24, 0x20,																// mov [ rsp + 20 ], rax
	0x48, 0x8B, 0x54, 0x24, 0x48,																// mov rdx, [ rsp + 48 ]
	0x48, 0x8B, 0x4C, 0x24, 0x40,																// mov rcx, [ rsp + 40 ]
	0xFF, 0x54, 0x24, 0x20,																			// call qword ptr [ rsp + 20 ]
	0x48, 0x83, 0xC4, 0x38,																			// add rsp, 38
	0xC3,																												// retn
};

NTSTATUS GetDriverObject( const wchar_t* const object_name, DRIVER_OBJECT** driver_object )
{
	auto status = STATUS_SUCCESS;

	if( !object_name )
	{
		TRACE( "%s: object_name is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER_1;
	}
	
	if( !driver_object )
	{
		TRACE( "%s: driver_object is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER_2;
	}

	UNICODE_STRING device_name = { };
	RtlInitUnicodeString( &device_name, object_name );

	FILE_OBJECT* file_object = nullptr;
	DEVICE_OBJECT* device_object = nullptr;

	status = IoGetDeviceObjectPointer( &device_name, FILE_READ_DATA, &file_object, &device_object );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: IoGetDeviceObjectPointer( '%S' ) error! (0x%08X)", __FUNCTION__, object_name, status );
		return status;
	}

	/*if( file_object )
	{
		ObDereferenceObject( file_object );
	}*/

	if( !device_object )
	{
		TRACE( "%s: device_object is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_ADDRESS;
	}

	*driver_object = device_object->DriverObject;

	if( !( *driver_object ) )
	{
		TRACE( "%s: *driver_object is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_ADDRESS;
	}

	return status;
}

NTSTATUS GetDriverDispatch( const DRIVER_OBJECT* const driver_object, PDRIVER_DISPATCH* driver_dispatch )
{
	auto status = STATUS_SUCCESS;

	if( !driver_object )
	{
		TRACE( "%s: driver_object is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER_1;
	}

	if( !driver_dispatch )
	{
		TRACE( "%s: driver_dispatch is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_PARAMETER_2;
	}

	*driver_dispatch = driver_object->MajorFunction[ IRP_MJ_DEVICE_CONTROL ];

	if( !( *driver_dispatch ) )
	{
		TRACE( "%s: *driver_dispatch is not valid!", __FUNCTION__ );
		return status = STATUS_INVALID_ADDRESS;
	}

	return status;
}

NTSTATUS HookDispatch( const wchar_t* const object_name )
{
	auto status = STATUS_SUCCESS;

	if( IsDispatchHooked() )
	{
		TRACE( "%s: Dispatch is already hooked!", __FUNCTION__ );
		return status;
	}

	status = GetDriverObject( object_name, &g_driver_object );
	TRACE( "g_driver_object = '0x%016llX'", memory::ToAddress( g_driver_object ) );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: GetDriverObject( '%S' ) error! (0x%08X)", __FUNCTION__, object_name, status );
		return status;
	}

	status = GetDriverDispatch( g_driver_object, &g_driver_dispatch );
	TRACE( "g_driver_dispatch = '0x%016llX'", memory::ToAddress( g_driver_dispatch ) );

	if( !NT_SUCCESS( status ) )
	{
		TRACE( "%s: GetDriverDispatch( '0x%016llX' ) error! (0x%08X)", __FUNCTION__, memory::ToAddress( g_driver_dispatch ), status );
		return status;
	}

	g_driver_data_section = win32::GetImageSection( memory::ToAddress( g_driver_object->DriverStart ), HASH( ".data" ) );

	if( !g_driver_data_section )
	{
		TRACE( "%s: win32::GetImageSection( '0x%016llX', '.data' ) error!", __FUNCTION__, memory::ToAddress( g_driver_object->DriverStart ) );
		return status = STATUS_NOT_FOUND;
	}

	const auto driver_data_begin = memory::ToAddress( g_driver_object->DriverStart ) + g_driver_data_section->VirtualAddress;
	TRACE( "driver_data_begin = '0x%016llX'", driver_data_begin );

	const auto driver_data_end = memory::ToAddress( g_driver_object->DriverStart ) + g_driver_data_section->VirtualAddress + g_driver_data_section->Misc.VirtualSize;
	TRACE( "driver_data_end = '0x%016llX'", driver_data_end );
	
	for( auto address = driver_data_begin; address < driver_data_end; address++ )
	{
		if( *reinterpret_cast< std::uint8_t* >( address ) == 0 )
		{
			g_driver_data_hook_size++;
		}
		else
		{
			g_driver_data_hook = 0;
			g_driver_data_hook_size = 0;
		}

		if( g_driver_data_hook_size >= sizeof( g_driver_dispatch_hook ) )
		{
			g_driver_data_hook = address;
			break;
		}
	}

	// const auto driver_data_hook = driver_data_end - sizeof( g_driver_dispatch_hook );
	TRACE( "g_driver_data_hook = '0x%016llX'", g_driver_data_hook );
	TRACE( "g_driver_data_hook_size = '0x%016llX'", g_driver_data_hook_size );

	if( !g_driver_data_hook )
	{
		TRACE( "%s: g_driver_data_hook is not valid!", __FUNCTION__ );
		return status = STATUS_NOT_FOUND;
	}

	win32::PageData driver_section_page_data = { };

	if( !win32::GetPageData( driver_data_begin, &driver_section_page_data ) )
	{
		TRACE( "%s: win32::GetPageData( '0x%016llX' ) error!", __FUNCTION__, driver_data_begin );
		return status = STATUS_NOT_FOUND;
	}

	if( !driver_section_page_data.m_pte )
	{
		TRACE( "%s: driver_section_page_data.m_pte = '0x%016llX' is not valid!", __FUNCTION__, memory::ToAddress( driver_section_page_data.m_pte ) );
		return status = STATUS_INVALID_ADDRESS;
	}

	// 
	// prepare hook
	// 
	*reinterpret_cast< void** >( g_driver_dispatch_hook + 16 ) = &io::DeviceControl;

	// 
	// make end of '.data' executable
	// 
	driver_section_page_data.m_pte->x64.Page4Kb.NX = false;

	// 
	// copy hook to the end of '.data' section
	// 
	std::memcpy( memory::ToPointer( g_driver_data_hook ), g_driver_dispatch_hook, sizeof( g_driver_dispatch_hook ) );

	// 
	// change device control function
	// 
	g_driver_object->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = reinterpret_cast< PDRIVER_DISPATCH >( g_driver_data_hook );

	// 
	// return success
	// 
	return status;
}

NTSTATUS UnhookDispatch()
{
	auto status = STATUS_SUCCESS;

	if( !IsDispatchHooked() )
	{
		TRACE( "%s: Dispatch is not hooked yet!", __FUNCTION__ );
		return status;
	}

	std::memset( memory::ToPointer( g_driver_data_hook ), 0, g_driver_data_hook_size );

	// 
	// restore device control function
	// 
	g_driver_object->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = g_driver_dispatch;

	// 
	// reset hook data
	// 
	g_driver_object = nullptr;
	g_driver_dispatch = nullptr;
	g_driver_data_section = nullptr;

	return status;
}

bool IsDispatchHooked()
{
	if( !g_driver_object )
	{
		return false;
	}

	if( !g_driver_dispatch )
	{
		return false;
	}

	if( !g_driver_data_section )
	{
		return false;
	}

	return true;
}

} // namespace atom