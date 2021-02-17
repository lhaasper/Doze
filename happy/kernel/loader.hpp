#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../core/no_copy.hpp"
#include "../core/no_move.hpp"

#include "../memory/operation.hpp"

#include "../win32/console.hpp"
#include "../win32/handle_guard.hpp"
#include "../win32/native.hpp"
#include "../win32/version.hpp"

#include "nt_system.hpp"

#if defined( CopyMemory )
	#undef CopyMemory
#endif // CopyMemory

constexpr bool is_space( char character )
{
	return ( character == ' ' );
}

constexpr bool is_question( char character )
{
	return ( character == '?' );
}

constexpr bool is_decimal( char character )
{
	return ( character >= '0' && character <= '9' );
}

constexpr bool is_hexadecimal_lower( char character )
{
	return ( character >= 'a' && character <= 'f' );
}

constexpr bool is_hexadecimal_upper( char character )
{
	return ( character >= 'A' && character <= 'F' );
}

constexpr int to_bits( char character )
{
	if( is_decimal( character ) )
		return static_cast< int >( character - '0' );
	else if( is_hexadecimal_lower( character ) )
		return static_cast< int >( character - 'a' + 0x0A );
	else if( is_hexadecimal_upper( character ) )
		return static_cast< int >( character - 'A' + 0x0A );
	else
		return static_cast< int >( -1 );
}

constexpr int to_bits_pair( int high, int low )
{
	return ( ( high & 0x0F ) << 4 | ( low & 0x0F ) );
}

template< typename T >
constexpr int to_byte( const T* data )
{
	return to_bits_pair( to_bits( data[ 0 ] ), to_bits( data[ 1 ] ) );
}

namespace atom::kernel
{

constexpr auto cx_device_code = 0x80862007;

struct COPY_MEMORY
{
	std::uint64_t m_case_number = 0;
	HORIZON_PAD( 0x0008 );
	std::uint64_t m_source = 0;
	std::uint64_t m_destination = 0;
	std::uint64_t m_size = 0;
};

struct GET_PHYSICAL_ADDRESS
{
	std::uint64_t m_case_number = 0;
	HORIZON_PAD( 0x0008 );
	std::uint64_t m_physical_address = 0;
	std::uint64_t m_virtual_address = 0;
};

struct MAP_IO_SPACE
{
	std::uint64_t m_case_number = 0;
	HORIZON_PAD( 0x0008 );
	std::uint64_t m_return_value = 0;
	std::uint64_t m_virtual_address = 0;
	std::uint64_t m_physical_address = 0;
	std::uint32_t m_size = 0;
};

struct UNMAP_IO_SPACE
{
	std::uint64_t m_case_number = 0;
	HORIZON_PAD( 0x0010 );
	std::uint64_t m_virtual_address = 0;
	HORIZON_PAD( 0x0008 );
	std::uint32_t m_size = 0;
};

class Loader : public core::NoCopy, public core::NoMove
{
public:
	Loader( const std::wstring& path, const std::wstring& name );
	~Loader();

public:
	bool Load();

	bool Reload();
	bool Unload();

public:
	bool CopyMemory( void* destination, const void* source, std::size_t size );

	bool GetPhysicalAddress( std::uintptr_t virtual_address, std::uintptr_t* physical_address );

	bool MapIoSpace( std::uintptr_t physical_address, std::uint32_t size, std::uintptr_t* virtual_address );
	bool UnmapIoSpace( std::uintptr_t virtual_address, std::uint32_t size );

	bool ReadMemory( std::uintptr_t address, void* data, std::size_t size );
	bool WriteMemory( std::uintptr_t address, const void* data, std::size_t size );

	bool WriteMemoryReadOnly( std::uintptr_t address, const void* data, std::size_t size );

public:
	std::uintptr_t GetSystemImage( const std::string& name );
	std::uintptr_t GetSystemImage( const std::wstring& name );
	std::uintptr_t GetImageExport( std::uintptr_t image, const std::string& name );

	bool InitializeUserCode();
	bool InitializeKernelCode();

protected:
	template< typename RequestType, typename ResponseType = void* >
	bool DeviceControl( RequestType* request, ResponseType* response = nullptr )
	{
		IO_STATUS_BLOCK io_status_block = { };

		const auto status = NtDeviceIoControlFile( m_device.Get(), nullptr, nullptr, nullptr, &io_status_block,
																							 static_cast< ULONG >( cx_device_code ),
																							 PVOID( request ),
																							 static_cast< ULONG >( request ? sizeof( RequestType ) : 0 ),
																							 PVOID( response ),
																							 static_cast< ULONG >( response ? sizeof( ResponseType ) : 0 ) );

		if( !NT_SUCCESS( status ) )
		{
			TRACE( "%s: NtDeviceIoControlFile( ... ) error! (0x%08X)", __FUNCTION__, status );
			return false;
		}

		return true;
	}

public:
	template< typename T >
	FORCEINLINE T Read( std::uintptr_t address )
	{
		T data = { };

		if( !ReadMemory( address, &data, sizeof( data ) ) )
			return { };

		return data;
	}

	#define READ_CALL_IMPL( Return, Name )					\
		FORCEINLINE Return Name( std::uintptr_t address )	\
		{													\
			return Read< Return >( address );				\
		}

	READ_CALL_IMPL( std::int8_t, ReadInt8 );
	READ_CALL_IMPL( std::int16_t, ReadInt16 );
	READ_CALL_IMPL( std::int32_t, ReadInt32 );
	READ_CALL_IMPL( std::int64_t, ReadInt64 );

	READ_CALL_IMPL( std::uint8_t, ReadUInt8 );
	READ_CALL_IMPL( std::uint16_t, ReadUInt16 );
	READ_CALL_IMPL( std::uint32_t, ReadUInt32 );
	READ_CALL_IMPL( std::uint64_t, ReadUInt64 );

	template< typename T >
	FORCEINLINE bool Write( std::uintptr_t address, const T& data )
	{
		return WriteMemory( address, &data, sizeof( data ) );
	}

	template< typename T = std::uintptr_t >
	T Rip( std::uintptr_t address, std::size_t opcode_count )
	{
		const auto disp = ReadInt32( address + opcode_count );
		return T( address + opcode_count + disp + sizeof( disp ) );
	}

	template< typename T = std::uintptr_t >
	T ScanRegion( const std::uint8_t* region_begin, const std::uint8_t* region_end, const char* signature )
	{
		auto scan_result = T( 0 );
		auto scan_compare = reinterpret_cast< const std::uint8_t* >( signature );

		const auto scan_begin = region_begin;
		const auto scan_end = region_end;

		for( auto scan_current = scan_begin; scan_current < scan_end; scan_current++ )
		{
			if( !scan_compare[ 0 ] )
				return scan_result;

			// if( MmIsAddressValid( PVOID( scan_current ) ) )
			{
				if( is_question( scan_compare[ 0 ] ) || scan_current[ 0 ] == to_byte( scan_compare ) )
				{
					if( !scan_result )
						scan_result = T( scan_current );

					if( !scan_compare[ 2 ] )
						return scan_result;

					const bool question[ 2 ] =
					{
						is_question( scan_compare[ 0 ] ),
						is_question( scan_compare[ 1 ] ),
					};

					if( ( question[ 0 ] && question[ 1 ] ) || ( !question[ 0 ] ) )
						scan_compare = ( scan_compare + 3 );
					else
						scan_compare = ( scan_compare + 2 );
				}
				else
				{
					scan_compare = reinterpret_cast< const std::uint8_t* >( signature );
					scan_result = T( 0 );
				}
			}
		}

		HORIZON_LOG( "%s: Couldn't find '%s' signature in [0x%016llX]->[0x%016llX]!", __FUNCTION__, signature, std::uintptr_t( scan_begin ), std::uintptr_t( scan_end ) );
		return T( 0 );
	}

	template< typename T = std::uintptr_t >
	T Scan( const char* signature )
	{
		const auto image_base = GetSystemImage( L"ntoskrnl.exe" );

		if( !image_base )
			return T( 0 );

		const auto image_dos_header = Read< IMAGE_DOS_HEADER >( image_base );

		if( image_dos_header.e_magic != IMAGE_DOS_SIGNATURE )
			return T( 0 );

		const auto image_nt_headers = Read< IMAGE_NT_HEADERS >( image_base + image_dos_header.e_lfanew );

		if( image_nt_headers.Signature != IMAGE_NT_SIGNATURE )
			return T( 0 );

		const auto image_size = image_nt_headers.OptionalHeader.SizeOfImage;

		if( !image_size )
			return T( 0 );

		const auto image_data = std::make_unique< std::uint8_t[] >( image_size );

		if( !ReadMemory( image_base, image_data.get(), image_size ) )
			return T( 0 );

		const auto region_begin = reinterpret_cast< const std::uint8_t* >( image_data.get() );
		const auto region_end = reinterpret_cast< const std::uint8_t* >( image_data.get() + image_size );

		const auto scan_result = ScanRegion< std::uintptr_t >( region_begin, region_end, signature );

		if( !scan_result )
			return T( 0 );

		return T( image_base + scan_result - std::uintptr_t( image_data.get() ) );
	}

	template< typename T, typename... ArgsT >
	bool Call( std::uintptr_t kernel_procedure, T *result, ArgsT... args )
	{
		if( !kernel_procedure )
			return false;

		if constexpr( std::is_same_v< T, void > )
		{
			UNREFERENCED_PARAMETER( result );
		}
		else
		{
			if( !result )
				return false;
		}

		std::array< std::uint8_t, 12 > kernel_procedure_jump =
		{
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// mov rax, 0x0000000000000000
			0xFF, 0xE0,																									// jmp rax
		};

		*reinterpret_cast< std::uintptr_t* >( &kernel_procedure_jump[ 2 ] ) = kernel_procedure;

		if( !WriteMemoryReadOnly( m_kernel_procedure_pointer, kernel_procedure_jump.data(), kernel_procedure_jump.size() ) )
			return false;

		if constexpr( std::is_same_v< T, void > )
		{
			const auto procedure = reinterpret_cast< void( NTAPI * )( ArgsT... ) >( m_user_procedure );
			procedure( args... );
		}
		else
		{
			const auto procedure = reinterpret_cast< T( NTAPI * )( ArgsT... ) >( m_user_procedure );
			*result = procedure( args... );
		}

		WriteMemoryReadOnly( m_kernel_procedure_pointer, m_kernel_procedure_code.data(), m_kernel_procedure_code.size() );
		return true;
	}
	
	template< typename T, typename... ArgsT >
	bool Call( const std::string &name, T *result, ArgsT... args )
	{
		const auto procedure = GetImageExport( m_image_ntoskrnl, name );

		if( !procedure )
			return false;

		return Call< T >( procedure, result, args... );
	}

	NtSystem& GetNtSystem();
	const NtSystem& GetNtSystem() const;

protected:
	PIMAGE_NT_HEADERS GetImageNtHeaders( std::uintptr_t image );

	bool MapHeader( std::uintptr_t image, std::uintptr_t data );
	bool MapSection( std::uintptr_t image, std::uintptr_t data );
	bool MapRelocation( std::uintptr_t image );
	bool MapImportDescriptor( std::uintptr_t image );
	bool MapImage( std::uintptr_t image, std::uintptr_t base );

	bool EraseSection( std::uintptr_t image, std::uintptr_t base );

	bool ExecuteImageEntry( std::uintptr_t image, std::uintptr_t base );

public:
	bool Map( const std::uint8_t* binary_data, std::size_t binary_size );

protected:
	bool Connect();
	void Disconnect();

	bool LoadInternal();
	bool UnloadInternal();

	bool CreateRegistry();

public:
	bool DestroyRegistry();

protected:
	NtSystem m_sub_system = { *this };

	win32::NtHandle m_device = { };

	std::wstring m_path = { };
	std::wstring m_object_name = { };
	std::wstring m_device_name = { };

	bool m_windows_7 = false;
	bool m_windows_10 = false;

	std::uintptr_t m_image_gdi32 = 0;
	std::uintptr_t m_user_procedure = 0;

	std::uintptr_t m_image_ntoskrnl = 0;
	std::uintptr_t m_image_win32k = 0;
	std::uintptr_t m_kernel_procedure_pointer = 0;
	std::uintptr_t m_kernel_procedure = 0;
	std::array< std::uint8_t, 12 > m_kernel_procedure_code = { };
};

} // namespace atom::kernel