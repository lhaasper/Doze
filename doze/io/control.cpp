#include "control.hpp"

#include "../atom/system_map_data.hpp"

#include "../constant/hash.hpp"
#include "../constant/string.hpp"

#include "../win32/sdk/portable_executable.hpp"

#include "../kernel/erase.hpp"
#include "../kernel/import.hpp"

#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../win32/critical_region.hpp"
#include "../win32/image.hpp"
#include "../win32/page.hpp"
#include "../win32/process.hpp"
#include "../win32/resource_lite.hpp"
#include "../win32/trace.hpp"

#include "../main/dispatch_hook.hpp"

#define MEM_IMAGE                   0x01000000

extern "C"
{

	NTKERNELAPI NTSTATUS NTAPI ZwProtectVirtualMemory( HANDLE ProcessHandle,
																										 PVOID* BaseAddress,
																										 SIZE_T* RegionSize,
																										 ULONG NewAccessProtection,
																										 ULONG* OldAccessProtection );

	void _driver_unload( std::uintptr_t data, ULONG tag );

}; // extern "C"

namespace atom::io
{

NTSTATUS OnQueryDriverVersion( const QueryDriverVersionRequest* const request, QueryDriverVersionResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		// doze 1.5
		response->m_major = 1;
		response->m_minor = 5;
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnQueryDriverSecure( const QueryDriverSecureRequest* const request, QueryDriverSecureResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		if( request->m_time_date_stamp )
		{
			const auto name_length = constant::GetLength( request->m_name );

			if( name_length )
			{
				UNICODE_STRING name = { };
				RtlInitUnicodeString( &name, request->m_name );

				status = kernel::EraseMmUnloadedDrivers( name );

				if( !NT_SUCCESS( status ) )
				{
					TRACE( "%s: kernel::EraseMmUnloadedDrivers( ... ) error! (0x%08X)", __FUNCTION__, status );
				}
				
				status = kernel::ErasePiDDBCacheTable( name, request->m_time_date_stamp );

				if( !NT_SUCCESS( status ) )
				{
					TRACE( "%s: kernel::ErasePiDDBCacheTable( ... ) error! (0x%08X)", __FUNCTION__, status );
				}
			}
			else
			{
				status = STATUS_INVALID_PARAMETER_3;
				TRACE( "%s: request->m_name is empty!", __FUNCTION__ );
			}
		}
		else
		{
			status = STATUS_INVALID_PARAMETER_2;
			TRACE( "%s: request->m_time_date_stamp is 0!", __FUNCTION__ );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnQueryDriverQuit( const QueryDriverQuitRequest* const request, QueryDriverQuitResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		status = UnhookDispatch();

		if( !NT_SUCCESS( status ) )
		{
			TRACE( "%s: UnhookDispatch() error! (0x%08X)", __FUNCTION__, status );
		}

		_driver_unload( g_map_data.m_base, 'TDmM' );
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnQueryUserProcess( const QueryUserProcessRequest* const request, QueryUserProcessResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();

			auto wow64_process = PsGetProcessWow64Process( process.GetProcess() );

			if( wow64_process )
			{
				response->m_wow64 = true;
				response->m_peb = reinterpret_cast< std::uintptr_t >( wow64_process );
			}
			else
			{
				response->m_wow64 = false;
				response->m_peb = reinterpret_cast< std::uintptr_t >( PsGetProcessPeb( process.GetProcess() ) );
			}

			response->m_base = reinterpret_cast< std::uintptr_t >( PsGetProcessSectionBaseAddress( process.GetProcess() ) );

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, status );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnQueryUserImage( const QueryUserImageRequest* const request, QueryUserImageResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();

			if( request->m_base )
			{
				if( !win32::GetUserImageName( process.GetProcess(), request->m_base, response->m_name ) )
				{
					status = STATUS_INVALID_IMAGE_FORMAT;
					TRACE( "%s: win32::GetUserImageName( ... ) error!", __FUNCTION__ );
				}
			}
			else
			{
				const auto name_length = constant::GetLength( request->m_name );

				if( name_length )
				{
					const auto image_base = win32::GetUserImage( process.GetProcess(), request->m_name );

					if( image_base )
					{
						const auto image_nt_headers = win32::GetImageNtHeaders( image_base );

						if( image_nt_headers )
						{
							response->m_base = image_base;
							response->m_size = static_cast< std::size_t >( image_nt_headers->OptionalHeader.SizeOfImage );
						}
						else
						{
							status = STATUS_INVALID_IMAGE_FORMAT;
							TRACE( "%s: win32::GetImageNtHeaders( ... ) error!", __FUNCTION__ );
						}
					}
					else
					{
						status = STATUS_NOT_FOUND;
						TRACE( "%s: win32::GetUserImage( ... ) error!", __FUNCTION__ );
					}
				}
				else
				{
					status = STATUS_INVALID_PARAMETER_2;
					TRACE( "%s: request->m_name is empty!", __FUNCTION__ );
				}
			}

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, status );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnUserMemoryCommit( const UserMemoryCommitRequest* const request, UserMemoryCommitResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();
			
			auto base_address = memory::ToPointer( request->m_address );
			auto region_size = static_cast< SIZE_T >( request->m_size );

			status = ZwAllocateVirtualMemory( ZwCurrentProcess(), &base_address, 0, &region_size, request->m_type, request->m_protection );

			if( NT_SUCCESS( status ) )
			{
				std::memset( base_address, 0, region_size );

				if( !request->m_no_execute )
				{
					auto page_begin = memory::ToAddress( base_address );
					auto page_end = page_begin + region_size;

					for( auto base = page_begin; base < page_end; base += PAGE_SIZE )
					{
						win32::PageData page = { };

						if( win32::GetPageData( base, &page ) )
						{
							if( page.m_pte )
							{
								// 
								// remove NoExecute bit
								// 
								page.m_pte->x64.Page4Kb.NX = false;
							}
						}
					}
				}

				response->m_address = memory::ToAddress( base_address );
				response->m_size = static_cast< std::size_t >( region_size );
			}
			else
			{
				TRACE( "%s: ZwAllocateVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
			}

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, status );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnUserMemoryFree( const UserMemoryFreeRequest* const request, UserMemoryFreeResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();

			auto base_address = memory::ToPointer( request->m_address );
			auto region_size = static_cast< SIZE_T >( request->m_size );

			status = ZwFreeVirtualMemory( ZwCurrentProcess(), &base_address, &region_size, request->m_type );

			if( !NT_SUCCESS( status ) )
			{
				TRACE( "%s: ZwFreeVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
			}

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, status );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS FuckedUpMemoryAllocate( UserMemoryCommitRequest* request, UserMemoryCommitResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();

			UNICODE_STRING nvapi64_unicode = { };
			RtlInitUnicodeString( &nvapi64_unicode, L"\\SystemRoot\\System32\\nvapi64.dll" );

			OBJECT_ATTRIBUTES file_attributes = { };
			InitializeObjectAttributes( &file_attributes, &nvapi64_unicode, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, nullptr, nullptr );

			HANDLE file_handle = nullptr;
			IO_STATUS_BLOCK io_status_block = { };
			
			status = ZwOpenFile( &file_handle, GENERIC_READ | SYNCHRONIZE, &file_attributes, &io_status_block, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE );

			if( NT_SUCCESS( status ) )
			{
				OBJECT_ATTRIBUTES section_attributes = { };
				InitializeObjectAttributes( &section_attributes, nullptr, OBJ_KERNEL_HANDLE, nullptr, nullptr );

				HANDLE section_handle = nullptr;

				status = ZwCreateSection( &section_handle, SECTION_MAP_READ | SECTION_MAP_WRITE, &section_attributes, nullptr, PAGE_READONLY, MEM_IMAGE, file_handle );

				if( NT_SUCCESS( status ) )
				{
					PVOID base_address = nullptr;
					SIZE_T view_size = 0;

					status = ZwMapViewOfSection( section_handle, ZwCurrentProcess(), &base_address, 0, view_size, nullptr, &view_size, ViewUnmap, 0, PAGE_READWRITE );

					if( NT_SUCCESS( status ) )
					{
						ULONG protection = PAGE_READWRITE;
						status = ZwProtectVirtualMemory( ZwCurrentProcess(), &base_address, &view_size, PAGE_READWRITE, &protection );

						if( NT_SUCCESS( status ) )
						{
							std::memset( base_address, 0, view_size );

							for( auto address = memory::ToAddress( base_address ); address < ( memory::ToAddress( base_address ) + view_size ); address += PAGE_SIZE )
							{
								win32::PageData page_data = { };

								if( win32::GetPageData( address, &page_data ) )
								{
									if( page_data.m_pte )
									{
										// 
										// make page executable & writeable
										// 
										page_data.m_pte->x64.Page4Kb.RW = true;
										page_data.m_pte->x64.Page4Kb.NX = false;
									}
								}
							}

							response->m_address = memory::ToAddress( base_address );
							response->m_size = view_size;
						}
						else
						{
							TRACE( "%s: ZwProtectVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
						}
					}
					else
					{
						TRACE( "%s: ZwMapViewOfSection( ... ) error! (0x%08X)", __FUNCTION__, status );
					}
				}
				else
				{
					TRACE( "%s: ZwCreateSection( ... ) error! (0x%08X)", __FUNCTION__, status );
				}
			}
			else
			{
				TRACE( "%s: ZwOpenFile( ... ) error! (0x%08X)", __FUNCTION__, status );
			}

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, process.GetStatus() );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request is not valid!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnUserMemoryRead( const UserMemoryReadRequest* const request, UserMemoryReadResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			auto from_process = process.GetProcess();
			auto from_address = memory::ToPointer( request->m_address );
			auto to_process = PsGetCurrentProcess();
			auto to_address = memory::ToPointer( request->m_data );
			auto buffer_size = request->m_size;
			auto number_of_bytes_copied = static_cast< SIZE_T >( 0 );

			status = MmCopyVirtualMemory( from_process, from_address, to_process, to_address, buffer_size, KernelMode, &number_of_bytes_copied );

			if( NT_SUCCESS( status ) )
			{
				response->m_size = static_cast< std::size_t >( number_of_bytes_copied );
			}
			else
			{
				TRACE( "%s: MmCopyVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
			}
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, process.GetStatus() );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnUserMemoryWrite( const UserMemoryWriteRequest* const request, UserMemoryWriteResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			auto from_process = PsGetCurrentProcess();
			auto from_address = memory::ToPointer( request->m_data );
			auto to_process = process.GetProcess();
			auto to_address = memory::ToPointer( request->m_address );
			auto buffer_size = request->m_size;
			auto number_of_bytes_copied = static_cast< SIZE_T >( 0 );

			status = MmCopyVirtualMemory( from_process, from_address, to_process, to_address, buffer_size, KernelMode, &number_of_bytes_copied );

			if( NT_SUCCESS( status ) )
			{
				response->m_size = static_cast< std::size_t >( number_of_bytes_copied );
			}
			else
			{
				TRACE( "%s: MmCopyVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
			}
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, process.GetStatus() );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

NTSTATUS OnUserMemoryQuery( const UserMemoryQueryRequest* const request, UserMemoryQueryResponse* response )
{
	auto status = STATUS_SUCCESS;

	if( request->IsValid() )
	{
		win32::Process process( request->m_process_id );

		if( process.IsValid() )
		{
			process.Attach();

			auto current_process = NtCurrentProcess();
			auto base_address = memory::ToPointer( request->m_address );

			SIZE_T return_length = 0;

			MEMORY_BASIC_INFORMATION information = { };
			status = ZwQueryVirtualMemory( current_process, base_address, MemoryBasicInformation, &information, sizeof( information ), &return_length );

			if( NT_SUCCESS( status ) )
			{
				auto destination = memory::ToAddress( response ) + FIELD_OFFSET( UserMemoryQueryResponse, m_base_address );
				std::memcpy( memory::ToPointer( destination ), &information, sizeof( information ) );
			}
			else
			{
				TRACE( "%s: ZwQueryVirtualMemory( ... ) error! (0x%08X)", __FUNCTION__, status );
			}

			process.Detach();
		}
		else
		{
			status = process.GetStatus();
			TRACE( "%s: process.IsValid() error! (0x%08X)", __FUNCTION__, process.GetStatus() );
		}
	}
	else
	{
		status = STATUS_INVALID_PARAMETER_1;
		TRACE( "%s: request->IsValid() error!", __FUNCTION__ );
	}

	response->m_status = status;
	return status;
}

void* GetSystemAddressForMdl( PMDL descriptor, ULONG priority )
{
	if( descriptor->MdlFlags & ( MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL ) )
	{
		return descriptor->MappedSystemVa;
	}

	return MmMapLockedPagesSpecifyCache( descriptor, KernelMode, MmNonCached, nullptr, FALSE, priority );
}

NTSTATUS DispatchDeviceControl( DEVICE_OBJECT* device_object, IRP* irp )
{
	UNREFERENCED_PARAMETER( device_object );

	auto status = STATUS_SUCCESS;
	auto information = static_cast< std::uint64_t >( 0 );

	const auto io_stack = IoGetCurrentIrpStackLocation( irp );

	if( io_stack )
	{
		const auto io_request_size = io_stack->Parameters.DeviceIoControl.InputBufferLength;
		const auto io_response_size = io_stack->Parameters.DeviceIoControl.OutputBufferLength;

		if( io_request_size >= sizeof( BaseRequest ) && io_response_size >= sizeof( BaseResponse ) )
		{
			const auto io_request_data = irp->AssociatedIrp.SystemBuffer;

			if( io_request_data )
			{
				auto io_response_data = GetSystemAddressForMdl( irp->MdlAddress, NormalPagePriority );

				if( io_response_data )
				{
					switch( io_stack->MajorFunction )
					{
						case IRP_MJ_DEVICE_CONTROL:
						{
							const auto io_operation = static_cast< Operation >( io_stack->Parameters.DeviceIoControl.IoControlCode );

							switch( io_operation )
							{
								case Operation::QueryDriverVersion:
								{
									const auto request = static_cast< const QueryDriverVersionRequest* >( io_request_data );

									if( io_request_size >= sizeof( QueryDriverVersionRequest ) )
									{
										QueryDriverVersionResponse response = { SignatureResponse };
										
										status = OnQueryDriverVersion( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::QueryDriverSecure:
								{
									const auto request = static_cast< const QueryDriverSecureRequest* >( io_request_data );

									if( io_request_size >= sizeof( QueryDriverSecureRequest ) )
									{
										QueryDriverSecureResponse response = { SignatureResponse };

										status = OnQueryDriverSecure( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::QueryDriverQuit:
								{
									const auto request = static_cast< const QueryDriverQuitRequest* >( io_request_data );

									if( io_request_size >= sizeof( QueryDriverQuitRequest ) )
									{
										QueryDriverQuitResponse response = { SignatureResponse };

										status = OnQueryDriverQuit( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::QueryUserProcess:
								{
									const auto request = static_cast< const QueryUserProcessRequest* >( io_request_data );

									if( io_request_size >= sizeof( QueryUserProcessRequest ) )
									{
										QueryUserProcessResponse response = { SignatureResponse };

										status = OnQueryUserProcess( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::QueryUserImage:
								{
									const auto request = static_cast< const QueryUserImageRequest* >( io_request_data );

									if( io_request_size >= sizeof( QueryUserImageRequest ) )
									{
										QueryUserImageResponse response = { SignatureResponse };

										status = OnQueryUserImage( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::UserMemoryCommit:
								{
									const auto request = static_cast< UserMemoryCommitRequest* >( io_request_data );

									if( io_request_size >= sizeof( UserMemoryCommitRequest ) )
									{
										UserMemoryCommitResponse response = { SignatureResponse };

										// status = OnUserMemoryCommit( request, &response );
										status = FuckedUpMemoryAllocate( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::UserMemoryFree:
								{
									const auto request = static_cast< const UserMemoryFreeRequest* >( io_request_data );

									if( io_request_size >= sizeof( UserMemoryFreeRequest ) )
									{
										UserMemoryFreeResponse response = { SignatureResponse };

										status = OnUserMemoryFree( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::UserMemoryRead:
								{
									const auto request = static_cast< const UserMemoryReadRequest* >( io_request_data );

									if( io_request_size >= sizeof( UserMemoryReadRequest ) )
									{
										UserMemoryReadResponse response = { SignatureResponse };

										status = OnUserMemoryRead( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::UserMemoryWrite:
								{
									const auto request = static_cast< const UserMemoryWriteRequest* >( io_request_data );

									if( io_request_size >= sizeof( UserMemoryWriteRequest ) )
									{
										UserMemoryWriteResponse response = { SignatureResponse };

										status = OnUserMemoryWrite( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
								case Operation::UserMemoryQuery:
								{
									const auto request = static_cast< const UserMemoryQueryRequest* >( io_request_data );

									if( io_request_size >= sizeof( UserMemoryQueryRequest ) )
									{
										UserMemoryQueryResponse response = { SignatureResponse };

										status = OnUserMemoryQuery( request, &response );
										information = sizeof( response );

										std::memcpy( io_response_data, &response, sizeof( response ) );
									}

									break;
								}
							}
							break;
						}
					}
				}
			}
		}
	}

	if( information )
	{
		irp->IoStatus.Status = status;
		irp->IoStatus.Information = information;

		IoCompleteRequest( irp, IO_NO_INCREMENT );
	}
	else
	{
		status = STATUS_NOT_IMPLEMENTED;
	}

	return status;
}

/*NTSTATUS DispatchDeviceControl( DEVICE_OBJECT* device_object, IRP* irp )
{
	auto status = STATUS_SUCCESS;
	auto information = static_cast< std::uint64_t >( 0 );

	STATUS_NOT_IMPLEMENTED

	auto irp_stack = IoGetCurrentIrpStackLocation( irp );

	if( !irp_stack )
	{
		return g_driver_dispatch( device_object, irp );
	}

	auto io_request_data = irp->AssociatedIrp.SystemBuffer;
	auto io_request_size = irp_stack->Parameters.DeviceIoControl.InputBufferLength;

	if( !io_request_data || !io_request_size )
	{
		return g_driver_dispatch( device_object, irp );
	}

	auto irp_descriptor = irp->MdlAddress;

	if( !irp_descriptor )
	{
		return g_driver_dispatch( device_object, irp );
	}

	auto io_response_data = GetSystemAddressForMdl( irp_descriptor, NormalPagePriority );
	auto io_response_size = irp_stack->Parameters.DeviceIoControl.OutputBufferLength;

	if( !io_response_data || !io_response_size )
	{
		return g_driver_dispatch( device_object, irp );
	}

	auto io_control_code = irp_stack->Parameters.DeviceIoControl.IoControlCode;

	switch( irp_stack->MajorFunction )
	{
		case IRP_MJ_DEVICE_CONTROL:
		{
			switch( io_control_code )
			{
				case CodeQueryVersion:
				{
					const auto request = static_cast< const QuerySystemVersionRequest* >( io_request_data );

					if( io_request_size >= sizeof( QuerySystemVersionRequest ) )
					{
						QuerySystemVersionResponse response = { SignatureResponse };
						status = OnQueryVersion( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeExecuteSecurity:
				{
					auto request = static_cast< QuerySecureSystemRequest* >( io_request_data );

					if( io_request_size >= sizeof( QuerySecureSystemRequest ) )
					{
						QuerySecureSystemResponse response = { SignatureResponse };
						status = ExecuteSecurity( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeHookDispatch:
				{
					const auto request = static_cast< const HookDispatchRequest* >( io_request_data );

					if( io_request_size >= sizeof( HookDispatchRequest ) )
					{
						HookDispatchResponse response = { SignatureResponse };
						status = OnHookDispatch( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeUnhookDispatch:
				{
					const auto request = static_cast< const UnhookDispatchRequest* >( io_request_data );

					if( io_request_size >= sizeof( UnhookDispatchRequest ) )
					{
						UnhookDispatchResponse response = { SignatureResponse };
						status = OnUnhookDispatch( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeQueryProcessData:
				{
					auto request = static_cast< QueryUserProcessRequest* >( io_request_data );

					if( io_request_size >= sizeof( QueryUserProcessRequest ) )
					{
						QueryUserProcessResponse response = { SignatureResponse };
						status = ExecuteGetProcessInformation( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeQueryImageData:
				{
					auto request = static_cast< QueryUserImageRequest* >( io_request_data );

					if( io_request_size >= sizeof( QueryUserImageRequest ) )
					{
						QueryUserImageResponse response = { SignatureResponse };
						status = ExecuteGetImageInformation( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeMemoryAllocate:
				{
					auto request = static_cast< UserMemoryCommitRequest* >( io_request_data );

					if( io_request_size >= sizeof( UserMemoryCommitRequest ) )
					{
						UserMemoryCommitResponse response = { SignatureResponse };
						status = OnMemoryAllocate( request, &response );
						// status = FuckedUpMemoryAllocate( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeMemoryFree:
				{
					auto request = static_cast< UserMemoryFreeRequest* >( io_request_data );

					if( io_request_size >= sizeof( UserMemoryFreeRequest ) )
					{
						UserMemoryFreeResponse response = { SignatureResponse };
						status = OnMemoryFree( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeMemoryRead:
				{
					auto request = static_cast< UserMemoryReadRequest* >( io_request_data );

					if( io_request_size >= sizeof( UserMemoryReadRequest ) )
					{
						UserMemoryReadResponse response = { SignatureResponse };
						status = OnUserMemoryRead( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeMemoryWrite:
				{
					auto request = static_cast< UserMemoryWriteRequest* >( io_request_data );

					if( io_request_size >= sizeof( UserMemoryWriteRequest ) )
					{
						UserMemoryWriteResponse response = { SignatureResponse };
						status = OnMemoryWrite( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeMapIoSpace:
				{
					auto request = static_cast< const MapIoSpaceRequest* >( io_request_data );

					if( io_request_size >= sizeof( MapIoSpaceRequest ) )
					{
						MapIoSpaceResponse response = { SignatureResponse };
						status = OnMapIoSpace( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				case CodeUnmapIoSpace:
				{
					auto request = static_cast< const UnmapIoSpaceRequest* >( io_request_data );

					if( io_request_size >= sizeof( UnmapIoSpaceRequest ) )
					{
						UnmapIoSpaceResponse response = { SignatureResponse };
						status = OnUnmapIoSpace( request, &response );
						information = sizeof( response );
						std::memcpy( io_response_data, &response, sizeof( response ) );
					}

					break;
				}
				default:
				{
					TRACE( "%s: Not supported control code '0x%08X'!", __FUNCTION__, io_control_code );
					break;
				}
			}
			break;
		}
		default:
		{
			TRACE( "%s: Not supported major function '%i'!", __FUNCTION__, irp_stack->MajorFunction );
			break;
		}
	}

	if( !information )
	{
		// 
		// not supported io control code
		// 
		return g_driver_dispatch( device_object, irp );
	}

	return DeviceControlComplete( irp, status, information );
}*/

NTSTATUS DeviceControl( DEVICE_OBJECT* device_object, IRP* irp )
{
	auto status = STATUS_SUCCESS;

	__try
	{
		status = DispatchDeviceControl( device_object, irp );

		if( status == STATUS_NOT_IMPLEMENTED )
		{
			status = g_driver_dispatch( device_object, irp );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		TRACE( "%s: Exception occurred! (0x%08X)", __FUNCTION__, GetExceptionCode() );
		status = g_driver_dispatch( device_object, irp );
	}

	return status;
}

NTSTATUS DeviceControlComplete( IRP* irp, NTSTATUS status, ULONG_PTR information )
{
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = information;

	IoCompleteRequest( irp, IO_NO_INCREMENT );
	return irp->IoStatus.Status;
}

} // namespace atom::io