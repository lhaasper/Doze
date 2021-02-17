#include "image_wipe_header.hpp"

#include "../constant/string.hpp"

#include "../memory/operation.hpp"

#include "../win32/image.hpp"
#include "../win32/trace.hpp"

#include <ntimage.h>

namespace atom
{

bool ImageWipeImportDescriptor( std::uintptr_t image )
{
	const auto image_data_directory = win32::GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_IMPORT );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: win32::GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_IMPORT ) error!", __FUNCTION__, image );
		return false;
	}

	auto image_import_descriptor = reinterpret_cast< IMAGE_IMPORT_DESCRIPTOR* >( image + image_data_directory->VirtualAddress );

	while( image_import_descriptor->Name )
	{
		auto thunk_data = reinterpret_cast< IMAGE_THUNK_DATA* >( image + image_import_descriptor->FirstThunk );

		while( thunk_data->u1.AddressOfData )
		{
			std::memset( thunk_data, 0, sizeof( IMAGE_THUNK_DATA ) );
			thunk_data++;
		}

		auto original_thunk_data = reinterpret_cast< IMAGE_THUNK_DATA* >( image + image_import_descriptor->OriginalFirstThunk );

		while( original_thunk_data->u1.AddressOfData )
		{
			std::memset( original_thunk_data, 0, sizeof( IMAGE_THUNK_DATA ) );
			original_thunk_data++;
		}

		std::memset( image_import_descriptor, 0, sizeof( IMAGE_IMPORT_DESCRIPTOR ) );
		image_import_descriptor++;
	}

	return true;
}

bool ImageWipeExportDirectory( std::uintptr_t image )
{
	const auto image_data_directory = win32::GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_EXPORT );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: win32::GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_EXPORT ) error!", __FUNCTION__, image );
		return false;
	}

	if( !image_data_directory->VirtualAddress ||
			!image_data_directory->Size )
	{
		return true;
	}

	const auto image_export_directory = reinterpret_cast< IMAGE_EXPORT_DIRECTORY* >( image + image_data_directory->VirtualAddress );

	const auto address_of_functions = reinterpret_cast< std::uint32_t* >( image + image_export_directory->AddressOfFunctions );
	const auto address_of_names = reinterpret_cast< std::uint32_t* >( image + image_export_directory->AddressOfNames );
	const auto address_of_name_ordinals = reinterpret_cast< std::uint16_t* >( image + image_export_directory->AddressOfNameOrdinals );

	for( std::uint32_t index = 0; index < image_export_directory->NumberOfFunctions; index++ )
	{
		const auto export_ordinal = address_of_name_ordinals[ index ];
		const auto export_procedure = address_of_functions[ export_ordinal ];

		if( !export_procedure )
		{
			continue;
		}

		const auto export_name = reinterpret_cast< char* >( image + address_of_names[ index ] );

		if( !memory::IsAddressValid( export_name ) )
		{
			continue;
		}

		const auto export_name_length = constant::GetLength( export_name );

		std::memset( export_name, 0, export_name_length * sizeof( char ) );
	}

	std::memset( address_of_functions, 0, sizeof( std::uint32_t ) * image_export_directory->NumberOfFunctions );
	std::memset( address_of_names, 0, sizeof( std::uint32_t ) * image_export_directory->NumberOfNames );
	std::memset( address_of_name_ordinals, 0, sizeof( std::uint16_t ) * image_export_directory->NumberOfFunctions );

	std::memset( image_export_directory, 0, sizeof( IMAGE_EXPORT_DIRECTORY ) );

	return true;
}

bool ImageWipeExceptionDirectory( std::uintptr_t image )
{
	const auto image_data_directory = win32::GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_EXCEPTION );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: win32::GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_EXCEPTION ) error!", __FUNCTION__, image );
		return false;
	}

	const auto image_exception_directory = reinterpret_cast< void* >( image + image_data_directory->VirtualAddress );
	std::memset( image_exception_directory, 0, image_data_directory->Size );

	return true;
}

bool ImageWipeDebugDirectory( std::uintptr_t image )
{
	const auto image_data_directory = win32::GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_DEBUG );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: win32::GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_DEBUG ) error!", __FUNCTION__, image );
		return false;
	}

	const auto image_debug_directory = reinterpret_cast< IMAGE_DEBUG_DIRECTORY* >( image + image_data_directory->VirtualAddress );
	std::memset( image_debug_directory, 0, image_data_directory->Size );

	return true;
}

bool ImageWipeTLSDirectory( std::uintptr_t image )
{
	const auto image_data_directory = win32::GetImageDataDirectory( image, IMAGE_DIRECTORY_ENTRY_TLS );

	if( !memory::IsAddressValid( image_data_directory ) )
	{
		TRACE( "%s: win32::GetImageDataDirectory( 0x%016llX, IMAGE_DIRECTORY_ENTRY_TLS ) error!", __FUNCTION__, image );
		return false;
	}

	const auto image_tls_directory = reinterpret_cast< IMAGE_TLS_DIRECTORY* >( image + image_data_directory->VirtualAddress );
	std::memset( image_tls_directory, 0, image_data_directory->Size );

	return true;
}

bool ImageWipeSectionHeader( std::uintptr_t image )
{
	const auto image_nt_headers = win32::GetImageNtHeaders( image );

	if( !memory::IsAddressValid( image_nt_headers ) )
	{
		TRACE( "%s: win32::GetImageNtHeaders( 0x%016llX ) error!", __FUNCTION__, image );
		return false;
	}

	const auto image_section_header = IMAGE_FIRST_SECTION( image_nt_headers );

	if( !memory::IsAddressValid( image_section_header ) )
	{
		TRACE( "%s: image_section_header is not valid!", __FUNCTION__ );
		return false;
	}

	for( std::uint16_t index = 0; index < image_nt_headers->FileHeader.NumberOfSections; index++ )
	{
		const auto image_section = &image_section_header[ index ];

		if( !memory::IsAddressValid( image_section ) )
		{
			continue;
		}

		std::memset( image_section, 0, sizeof( IMAGE_SECTION_HEADER ) );
	}

	return true;
}

bool ImageWipeHeader( std::uintptr_t image )
{
	const auto image_nt_headers = win32::GetImageNtHeaders( image );

	if( !memory::IsAddressValid( image_nt_headers ) )
	{
		TRACE( "%s: win32::GetImageNtHeaders( 0x%016llX ) error!", __FUNCTION__, image );
		return false;
	}

	TRACE( "image_nt_headers->OptionalHeader.SizeOfCode: 0x%08X", image_nt_headers->OptionalHeader.SizeOfCode );
	TRACE( "image_nt_headers->OptionalHeader.SizeOfImage: 0x%08X", image_nt_headers->OptionalHeader.SizeOfImage );
	TRACE( "image_nt_headers->OptionalHeader.SizeOfHeaders: 0x%08X", image_nt_headers->OptionalHeader.SizeOfHeaders );

	/*if( !ImageWipeImportDescriptor( image ) )
	{
		TRACE( "%s: ImageWipeImportDescriptor( 0x%016llX ) error!", __FUNCTION__, image );
	}*/

	if( !ImageWipeExportDirectory( image ) )
	{
		TRACE( "%s: ImageWipeExportDirectory( 0x%016llX ) error!", __FUNCTION__, image );
	}

	/*if( !ImageWipeExceptionDirectory( image ) )
	{
		TRACE( "%s: ImageWipeExceptionDirectory( 0x%016llX ) error!", __FUNCTION__, image );
	}*/

	if( !ImageWipeDebugDirectory( image ) )
	{
		TRACE( "%s: ImageWipeDebugDirectory( 0x%016llX ) error!", __FUNCTION__, image );
	}

	if( !ImageWipeTLSDirectory( image ) )
	{
		TRACE( "%s: ImageWipeTLSDirectory( 0x%016llX ) error!", __FUNCTION__, image );
	}

	if( !ImageWipeSectionHeader( image ) )
	{
		TRACE( "%s: ImageWipeSectionHeader( 0x%016llX ) error!", __FUNCTION__, image );
	}

	const auto image_base = memory::ToPointer( image );

	std::memset( image_base, 0, PAGE_SIZE );

	return true;
}

}