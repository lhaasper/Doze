#include "dump.hpp"

#include "../memory/macro.hpp"
#include "../memory/operation.hpp"
#include "../memory/scan.hpp"

#include "../io/packet.hpp"
#include "../io/driver_control.hpp"

#include <tlhelp32.h>

extern "C"
{
	__declspec( dllimport ) NTSTATUS NTAPI NtQueryVirtualMemory( _In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress, _In_ std::uint32_t MemoryInformationClass, _Out_writes_bytes_( MemoryInformationLength ) PVOID MemoryInformation, _In_ SIZE_T MemoryInformationLength, _Out_opt_ PSIZE_T ReturnLength );

};

namespace atom::win32
{

enum FUNCTION_TABLE_TYPE
{
	RF_SORTED = 0,
	RF_UNSORTED = 1,
	RF_CALLBACK = 2,
	RF_KERNEL_DYNAMIC = 3,
};

struct RTL_BALANCED_NODE
{
	union
	{
		RTL_BALANCED_NODE* Children[ 2 ] = { };
		struct
		{
			RTL_BALANCED_NODE* Left;
			RTL_BALANCED_NODE* Right;
		};
	};
	union
	{
		std::uint8_t Red : 1;
		std::uint8_t Balance : 2;
		std::uint64_t ParentValue = 0;
	};
};

struct DYNAMIC_FUNCTION_TABLE
{
	LIST_ENTRY ListEntry = { };
	IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable = nullptr;
	LARGE_INTEGER TimeStamp = { };
	std::uint64_t MinimumAddress = 0;
	std::uint64_t MaximumAddress = 0;
	std::uint64_t BaseAddress = 0;
	void* Callback = nullptr;
	void* Context = nullptr;
	wchar_t* OutOfProcessCallbackDll = nullptr;
	FUNCTION_TABLE_TYPE Type = RF_SORTED;
	std::uint32_t EntryCount = 0;
	RTL_BALANCED_NODE TreeNode = { };
};

struct INVERTED_FUNCTION_TABLE_ENTRY
{
	union
	{
		IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable = nullptr;
		DYNAMIC_FUNCTION_TABLE* DynamicTable;
	};
	void* ImageBase = nullptr;
	std::uint32_t SizeOfImage = 0;
	std::uint32_t SizeOfTable = 0;
};

struct INVERTED_FUNCTION_TABLE
{
	std::uint32_t CurrentSize = 0;
	std::uint32_t MaximumSize = 0;
	std::uint32_t Epoch = 0;
	std::uint8_t Overflow = 0;
	std::uint8_t Reserved[ 3 ] = { };
	INVERTED_FUNCTION_TABLE_ENTRY TableEntry[ 256 ] = { };
};

} // namespace atom::win32

namespace atom::win32
{

enum LDR_DLL_LOAD_REASON
{
	LoadReasonStaticDependency = 0,
	LoadReasonStaticForwarderDependency = 1,
	LoadReasonDynamicForwarderDependency = 2,
	LoadReasonDelayloadDependency = 3,
	LoadReasonDynamicLoad = 4,
	LoadReasonAsImageLoad = 5,
	LoadReasonAsDataLoad = 6,
	LoadReasonEnclavePrimary = 7,
	LoadReasonEnclaveDependency = 8,
	LoadReasonUnknown = -1,
};

#pragma warning( push )
#pragma warning( disable : 4201 )

struct LIST_ENTRY
{
	LIST_ENTRY* Flink = nullptr;	// 0x0000
	LIST_ENTRY* Blink = nullptr;	// 0x0008
};
// sizeof( PEB_LDR_DATA ) = 0x0010

struct LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks = { };
	LIST_ENTRY InMemoryOrderLinks = { };
	LIST_ENTRY InInitializationOrderLinks = { };
	void* DllBase = nullptr;
	void* EntryPoint = nullptr;
	std::uint32_t SizeOfImage = 0;
	std::uint8_t Reserved0[ 0x0004 ] = { };
	UNICODE_STRING FullDllName = { };
	UNICODE_STRING BaseDllName = { };
	union
	{
		std::uint8_t FlagGroup[ 4 ] = { };
		std::uint32_t Flags;
		struct
		{
			std::uint32_t PackagedBinary : 1; /* bit position: 0 */
			std::uint32_t MarkedForRemoval : 1; /* bit position: 1 */
			std::uint32_t ImageDll : 1; /* bit position: 2 */
			std::uint32_t LoadNotificationsSent : 1; /* bit position: 3 */
			std::uint32_t TelemetryEntryProcessed : 1; /* bit position: 4 */
			std::uint32_t ProcessStaticImport : 1; /* bit position: 5 */
			std::uint32_t InLegacyLists : 1; /* bit position: 6 */
			std::uint32_t InIndexes : 1; /* bit position: 7 */
			std::uint32_t ShimDll : 1; /* bit position: 8 */
			std::uint32_t InExceptionTable : 1; /* bit position: 9 */
			std::uint32_t ReservedFlags1 : 2; /* bit position: 10 */
			std::uint32_t LoadInProgress : 1; /* bit position: 12 */
			std::uint32_t LoadConfigProcessed : 1; /* bit position: 13 */
			std::uint32_t EntryProcessed : 1; /* bit position: 14 */
			std::uint32_t ProtectDelayLoad : 1; /* bit position: 15 */
			std::uint32_t ReservedFlags3 : 2; /* bit position: 16 */
			std::uint32_t DontCallForThreads : 1; /* bit position: 18 */
			std::uint32_t ProcessAttachCalled : 1; /* bit position: 19 */
			std::uint32_t ProcessAttachFailed : 1; /* bit position: 20 */
			std::uint32_t CorDeferredValidate : 1; /* bit position: 21 */
			std::uint32_t CorImage : 1; /* bit position: 22 */
			std::uint32_t DontRelocate : 1; /* bit position: 23 */
			std::uint32_t CorILOnly : 1; /* bit position: 24 */
			std::uint32_t ChpeImage : 1; /* bit position: 25 */
			std::uint32_t ReservedFlags5 : 2; /* bit position: 26 */
			std::uint32_t Redirected : 1; /* bit position: 28 */
			std::uint32_t ReservedFlags6 : 2; /* bit position: 29 */
			std::uint32_t CompatDatabaseProcessed : 1; /* bit position: 31 */
		};
	};
	std::uint16_t ObsoleteLoadCount = 0;
	std::uint16_t TlsIndex = 0;
	LIST_ENTRY HashLinks = { };
	std::uint32_t TimeDateStamp = 0;
	std::uint8_t Reserved1[ 0x0004 ] = { };
	void* EntryPointActivationContext = nullptr;
	void* Lock = nullptr;
	void* DdagNode = nullptr;
	LIST_ENTRY NodeModuleLink = { };
	void* LoadContext = nullptr;
	void* ParentDllBase = nullptr;
	void* SwitchBackContext = nullptr;
	RTL_BALANCED_NODE BaseAddressIndexNode = { };
	RTL_BALANCED_NODE MappingInfoIndexNode = { };
	std::uint64_t OriginalBase = 0;
	LARGE_INTEGER LoadTime = { };
	std::uint32_t BaseNameHashValue = 0;
	LDR_DLL_LOAD_REASON LoadReason = LoadReasonUnknown;
	std::uint32_t ImplicitPathOptions = 0;
	std::uint32_t ReferenceCount = 0;
	std::uint32_t DependentLoadFlags = 0;
	std::uint8_t SigningLevel = 0;
	std::uint8_t Reserved2[ 0x0003 ] = { };
};

struct PEB_LDR_DATA
{
	std::uint32_t Length = 0;													// 0x0000
	std::uint8_t Initialized = 0;											// 0x0004
	std::uint8_t Reserved0[ 0x0003 ] = { };						// 0x0005
	void* SsHandle = nullptr;													// 0x0008
	LIST_ENTRY InLoadOrderModuleList = { };						// 0x0010
	LIST_ENTRY InMemoryOrderModuleList = { };					// 0x0020
	LIST_ENTRY InInitializationOrderModuleList = { };	// 0x0030
	void* EntryInProgress = nullptr;									// 0x0040
	std::uint8_t ShutdownInProgress = 0;							// 0x0048
	std::uint8_t Reserved1[ 0x0007 ] = { };						// 0x0049
	void* ShutdownThreadId = nullptr;									// 0x0050
};
// sizeof( PEB_LDR_DATA ) = 0x0058

struct PEB
{
	std::uint8_t InheritedAddressSpace = 0;
	std::uint8_t ReadImageFileExecOptions = 0;
	std::uint8_t BeingDebugged = 0;
	std::uint8_t BitField = 0;
	void* Mutant = nullptr;
	void* ImageBaseAddress = nullptr;
	PEB_LDR_DATA* Ldr = nullptr;
	void* ProcessParameters = nullptr;
	void* SubSystemData = nullptr;
	void* ProcessHeap = nullptr;
	void* FastPebLock = nullptr;
	void* AtlThunkSListPtr = nullptr;
	void* IFEOKey = nullptr;
	void* CrossProcessFlags = nullptr;
	void* KernelCallbackTable = nullptr;
	std::uint32_t SystemReserved = 0;
	std::uint32_t AtlThunkSListPtr32 = 0;
	void* ApiSetMap = nullptr;
};

#pragma warning( pop )

} // namespace atom::win32

namespace atom
{

// 
// 4C 89 44 24 ? 48 89 54 24 ? 89 4C 24 08 56 57 B8
// 

namespace pe
{

struct ImageData
{
	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;
	std::wstring m_name = { };
};

struct PageData
{
	bool CanRead() const
	{
		constexpr auto protection = ( PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY );
		return !!( m_protect & protection );
	}

	bool CanWrite() const
	{
		constexpr auto protection = ( PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY );
		return !!( m_protect & protection );
	}

	bool CanExecute() const
	{
		constexpr auto protection = ( PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY );
		return !!( m_protect & protection );
	}

	bool IsGuard() const
	{
		return !!( m_protect & PAGE_GUARD );
	}

	bool IsNoCache() const
	{
		return !!( m_protect & PAGE_NOCACHE );
	}

	bool IsWriteCombine() const
	{
		return !!( m_protect & PAGE_WRITECOMBINE );
	}

	std::uintptr_t m_base = 0;
	std::size_t m_size = 0;
	std::uint32_t m_type = 0;
	std::uint32_t m_state = 0;
	std::uint32_t m_protect = 0;
};

std::vector< ImageData > EnumerateImageArray()
{
	auto& doze = io::DriverControl::Instance();

	std::vector< ImageData > output = { };

	if( doze.IsValid() )
	{
		const auto pointer_peb = doze.GetPeb();

		if( memory::IsAddressValid( pointer_peb ) )
		{
			const auto peb = doze.Read< win32::PEB >( pointer_peb );

			if( memory::IsAddressValid( peb.Ldr ) )
			{
				const auto peb_ldr_data = doze.Read< win32::PEB_LDR_DATA >( memory::ToAddress( peb.Ldr ) );
				const auto in_load_order_module_list = memory::ToAddress( peb.Ldr ) + FIELD_OFFSET( win32::PEB_LDR_DATA, InLoadOrderModuleList );

				if( memory::IsAddressValid( in_load_order_module_list ) )
				{
					for( auto list_entry = doze.Read< std::uintptr_t >( in_load_order_module_list );
							 list_entry != in_load_order_module_list;
							 list_entry = doze.Read< std::uintptr_t >( list_entry ) )
					{
						const auto pointer_ldr_data_table_entry = ( list_entry - FIELD_OFFSET( win32::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks ) );

						if( memory::IsAddressValid( pointer_ldr_data_table_entry ) )
						{
							const auto ldr_data_table_entry = doze.Read< win32::LDR_DATA_TABLE_ENTRY >( pointer_ldr_data_table_entry );

							const auto& base_name = ldr_data_table_entry.BaseDllName;

							if( base_name.Length && memory::IsAddressValid( base_name.Buffer ) )
							{
								const auto base_name_length = ( base_name.Length / sizeof( wchar_t ) );

								std::wstring name = { };
								name.resize( base_name_length + 1 );

								if( doze.ReadMemory( memory::ToAddress( base_name.Buffer ), name.data(), base_name.Length ) )
								{
									ImageData data = { };

									data.m_base = memory::ToAddress( ldr_data_table_entry.DllBase );
									data.m_size = static_cast< std::size_t >( ldr_data_table_entry.SizeOfImage );
									data.m_name = std::move( name );

									output.emplace_back( std::move( data ) );
								}
							}
						}
					}
				}
			}
		}
	}

	return std::move( output );
}

std::vector< PageData > EnumeratePageArray()
{
	auto& doze = io::DriverControl::Instance();

	std::vector< PageData > output = { };

	if( doze.IsValid() )
	{
		MEMORY_BASIC_INFORMATION information = { };

		for( std::uintptr_t address = 0; address < memory::MaximumUserAddress; address += information.RegionSize )
		{
			if( doze.UserMemoryQuery( address, &information ) )
			{
				PageData data = { };

				data.m_base = memory::ToAddress( information.BaseAddress );
				data.m_size = information.RegionSize;
				data.m_type = information.Type;
				data.m_state = information.State;
				data.m_protect = information.Protect;

				output.emplace_back( std::move( data ) );
			}
		}
	}

	return std::move( output );
}

bool IsPointerSuspicious( const void* pointer )
{
	const auto image_array = EnumerateImageArray();

	if( image_array.empty() )
	{
		return false;
	}

	const auto address = memory::ToAddress( pointer );

	for( const auto& image : image_array )
	{
		if( address >= image.m_base && address < ( image.m_base + image.m_size ) )
		{
			return false;
		}
	}

	return true;
}

bool IsPointerSuspicious( std::uintptr_t address )
{
	const auto pointer = memory::ToConstantPointer( address );
	return IsPointerSuspicious( pointer );
}

bool DumpToDisk( const PageData& page, const std::wstring& name )
{
	auto& doze = io::DriverControl::Instance();

	auto data = std::make_unique< char[] >( page.m_size );

	if( !doze.ReadMemory( page.m_base, data.get(), page.m_size ) )
	{
		return false;
	}

	std::ofstream output( name, std::ios::app | std::ios::binary );

	if( output.is_open() )
	{
		output.write( data.get(), page.m_size );
		output.close();
		return true;
	}

	return false;
}

} // namespace pe

#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

PBYTE findPattern( const PBYTE rangeStart, const PBYTE rangeEnd, const char* pattern )
{
	const unsigned char* pat = reinterpret_cast< const unsigned char* >( pattern );
	PBYTE firstMatch = 0;
	for( PBYTE pCur = rangeStart; pCur < rangeEnd; ++pCur )
	{
		if( *( PBYTE )pat == ( BYTE )'\?' || *pCur == getByte( pat ) )
		{
			if( !firstMatch )
			{
				firstMatch = pCur;
			}
			pat += ( *( PWORD )pat == ( WORD )'\?\?' || *( PBYTE )pat != ( BYTE )'\?' ) ? 2 : 1;
			if( !*pat )
			{
				return firstMatch;
			}
			pat++;
			if( !*pat )
			{
				return firstMatch;
			}
		}
		else if( firstMatch )
		{
			pCur = firstMatch;
			pat = reinterpret_cast< const unsigned char* >( pattern );
			firstMatch = 0;
		}
	}
	return NULL;
}

namespace be
{

// 
// these are checked specifically
// 
constexpr std::uint32_t g_time_stamps[] =
{
	0x5B12C900,
	0x5A180C35,
	0xFC9B9325,
	0x456CED13,
	0x46495AD9,
	0x47CDEE2B,
	0x469FF22E,
	0x48EC3AD7,
	0x5A8E6020,
	0x55C85371,
};

// 
// these are reported if present
// 
constexpr std::uint32_t g_blacklisted_time_stamps[] =
{
	0x5D728445,
	0x5E87A1D2,
	0x5E93BF48,
};

constexpr std::size_t g_region_size_array[] =
{
	0x11000,
	0x13000,
	0x14000,
	0x1A000,
	0x2E000,
	0x3E000,
	0x46000,
	0xBF000,
	0x174000,
	0xC1000,
	0x1B000,
	0x25000,
	0xA1000,
	0x16B000,
	0x30000,
	0x12000,
	0x89000,
	0x182000,
	0xA2000,
	0x17C000,
	0x51000,
	0x26000,
	0x17E000,
	0xA4000,
	0x79000,
	0x53000,
	0x17F000,
	0x7C000,
	0xA5000,
	0x1C000,
	0x186000,
	0xA6000,
	0x80000,
	0x32000,
	0x16000,
	0x18A000,
	0x82000,
	0x2D000,
	0x15000,
	0x18B000,
	0x7B000,
	0x55000,
	0xA7000,
	0x49000,
	0x33000,
	0x27000,
	0x18D000,
	0x4A000,
	0x56000,
	0x191000,
	0xF3000,
	0x57000,
	0x194000,
	0xA8000,
	0x34000,
	0x28000,
};

IMAGE_DOS_HEADER* get_dos_header( void* image )
{
	if( !image )
	{
		return nullptr;
	}

	auto image_dos_header = reinterpret_cast< IMAGE_DOS_HEADER* >( image );

	if( image_dos_header->e_magic != IMAGE_DOS_SIGNATURE )
	{
		return nullptr;
	}

	return image_dos_header;
}

IMAGE_NT_HEADERS* get_nt_headers( void* image )
{
	auto image_dos_header = get_dos_header( image );

	if( !image_dos_header )
	{
		return nullptr;
	}

	if( !image_dos_header->e_lfanew )
	{
		return nullptr;
	}

	auto image_nt_headers = reinterpret_cast< IMAGE_NT_HEADERS* >( reinterpret_cast< std::uintptr_t >( image ) + image_dos_header->e_lfanew );

	if( image_nt_headers->Signature != IMAGE_NT_SIGNATURE )
	{
		return nullptr;
	}

	return image_nt_headers;
}

void run_checks()
{
	SIZE_T return_length = 0;
	MEMORY_BASIC_INFORMATION information = { };

	for( std::uint64_t address = 0;
			 NT_SUCCESS( NtQueryVirtualMemory( HANDLE( -1 ), PVOID( address ), 0, &information, sizeof( information ), &return_length ) );
			 address = std::uint64_t( information.BaseAddress ) + information.RegionSize )
	{
		const auto can_execute = ( information.Protect == PAGE_EXECUTE );
		const auto can_execute_read = ( information.Protect == PAGE_EXECUTE_READ );
		const auto can_execute_read_write = ( information.Protect == PAGE_EXECUTE_READWRITE );

		const auto is_commit = ( information.State == MEM_COMMIT );
		const auto is_executable = ( can_execute || can_execute_read || can_execute_read_write );

		const auto is_be_code = ( std::uint64_t( information.BaseAddress ) > std::uint64_t( &run_checks ) ||
															( std::uint64_t( information.BaseAddress ) + information.RegionSize ) <= std::uint64_t( &run_checks ) );

		const auto is_private = ( information.Type == MEM_PRIVATE );

		// 
		// memory state is MEM_COMMIT and is executable and is not our shellcode and...
		// not private OR: 
		//		not PAGE_EXECUTE_READ or has RegionSize that is not whitelisted and not PAGE_EXECUTE_READWRITE or has RegionSize that's not whitelisted
		// 
		if( ( is_commit ) &&
				( is_executable ) &&
				( is_be_code  ) &&
				( information.Type != MEM_PRIVATE ||
					( information.Protect != PAGE_EXECUTE_READ ||
						information.RegionSize != 0x11000 &&
						information.RegionSize != 0x13000 &&
						information.RegionSize != 0x14000 &&
						information.RegionSize != 0x1A000 &&
						information.RegionSize != 0x2E000 &&
						information.RegionSize != 0x3E000 &&
						information.RegionSize != 0x46000 &&
						( information.RegionSize < 0xC7000 || information.RegionSize > 0x130000 ) &&
						( information.RegionSize < 0x70000 || information.RegionSize > 0x9D000 ) &&
						( information.RegionSize < 0x40000 || information.RegionSize > 0x50000 ) &&
						information.RegionSize != 0xAF000 ) &&
					( information.Protect != PAGE_EXECUTE_READWRITE ||
						information.RegionSize != 0x12000 &&
						information.RegionSize != 0x18000 &&
						information.RegionSize != 0x20000 ) ) )
		{
			
		}

		auto NSPStartup = GetProcAddress( HMODULE( information.BaseAddress ), "NSPStartup" );

		if( NSPStartup )
		{
			auto image_nt_headers = get_nt_headers( information.BaseAddress );
			auto image_optional_header = &image_nt_headers->OptionalHeader;

			if( image_optional_header->DataDirectory[ IMAGE_DIRECTORY_ENTRY_SECURITY ].Size == 0x1B20 ||
					image_optional_header->DataDirectory[ IMAGE_DIRECTORY_ENTRY_SECURITY ].Size == 0xE70 ||
					image_optional_header->DataDirectory[ IMAGE_DIRECTORY_ENTRY_SECURITY ].Size == 0x1A38 || 
					image_nt_headers->FileHeader.TimeDateStamp >= 0x5C600000 && image_nt_headers->FileHeader.TimeDateStamp < 0x5C700000 )
			{

			}
		}
	}
}

} // namespace be

bool DumpBattlEye()
{
	const auto window = FindWindowW( L"UnrealWindow", nullptr );

	if( !window )
	{
		TRACE( "%s: FindWindowW( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	DWORD process_id = 0;
	const auto thread_id = GetWindowThreadProcessId( window, &process_id );

	if( !process_id || !thread_id )
	{
		TRACE( "%s: GetWindowThreadProcessId( ... ) error! (0x%08X)", __FUNCTION__, GetLastError() );
		return false;
	}

	auto& doze = io::DriverControl::Instance();

	doze.SetWindow( window );
	doze.SetThreadId( thread_id );
	doze.SetProcessId( process_id );

	// while( true )
	{
		const auto page_array = pe::EnumeratePageArray();

		if( !page_array.empty() )
		{
			for( const auto& page : page_array )
			{
				if( page.CanExecute() )
				{
					if( pe::IsPointerSuspicious( page.m_base ) )
					{
						// const auto page_data = std::make_unique< std::uint8_t[] >( page.m_size );
						
						// if( doze.ReadMemory( page.m_base, page_data.get(), page.m_size ) )
						{
							// const auto scan_begin = page_data.get();
							// const auto scan_end = scan_begin + page.m_size;

							// const auto scan_result = std::uintptr_t( findPattern( scan_begin, scan_end, "4C 89 44 24 ? 48 89 54 24 ? 89 4C 24 08 56 57 B8" ) );
							// TRACE( "scan_result = '0x%016llX'", scan_result );

							// if( scan_result )
							{
								// const auto result = scan_result - memory::ToAddress( page_data.get() ) + page.m_base;
								// TRACE( "BattlEye = '0x%016llX'", result );

								const auto page_begin = page.m_base;
								const auto page_end = ( page.m_base + page.m_size );

								wchar_t page_name[ 256 ] = { };
								swprintf_s( page_name, L"page_0x%016llX_0x%016llX.bin", page_begin, page_end );

								if( !pe::DumpToDisk( page, page_name ) )
								{
									TRACE( "%s: pe::DumpToDisk( ... ) error!", __FUNCTION__ );
								}

								TRACE( "base = '0x%016llX'", page.m_base );
								TRACE( "size = '0x%016llX'", page.m_size );
								TRACE( "type = '0x%08X'", page.m_type );
								TRACE( "state = '0x%08X'", page.m_state );
								TRACE( "protect = '0x%08X'", page.m_protect );
								TRACE_SEPARATOR();

								// return true;
							}
						}
					}
				}
			}
		}

		std::this_thread::sleep_for( 100ms );
	}

	return false;
}

} // namespace atom