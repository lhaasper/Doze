#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

namespace atom::win32
{

// 
// 1 << 12 == 4096
// 
#define PFN_TO_PAGE( pfn )			( ( pfn ) << 12 )
#define PAGE_TO_PFN( page )			( ( page ) >> 12 )

// 
// cr3 in pae : pdp at 5:31
// 
#define PFN_TO_PDP_PAE( pfn )		( ( pfn ) << 5 )

// 
// set 1-byte structure packing
// 
#pragma pack( push, 1 )

union VIRTUAL_ADDRESS
{
	std::uint64_t Value = 0;															// 
	union
	{
		std::uint64_t Value = 0;														// 
		struct
		{
			std::uint64_t Reserved0 : 30;											// 
			std::uint64_t PageDirectoryPointerOffset : 9;			// 
			std::uint64_t PageMapLevel4Offset : 9;						// 
			std::uint64_t SignExtend : 16;										// 
		} Generic;																					// 
		union
		{
			struct
			{
				std::uint64_t Reserved0 : 21;										// 
				std::uint64_t PageDirectoryOffset : 9;					// 
				std::uint64_t PageDirectoryPointerOffset : 9;		// 
				std::uint64_t PageMapLevel4Offset : 9;					// 
				std::uint64_t SignExtend : 16;									// 
			} Generic;																				// to determine whether page size is 4kb or 2mb
			struct
			{
				std::uint64_t PageOffset : 12;									// byte offset into the physical page
				std::uint64_t PageTableOffset : 9;							// index into the 512-entry page table
				std::uint64_t PageDirectoryOffset : 9;					// index into the 512-entry page-directory table
				std::uint64_t PageDirectoryPointerOffset : 9;		// index into the 512-entry page-directory pointer table
				std::uint64_t PageMapLevel4Offset : 9;					// index into the 512-entry page-map level-4 table
				std::uint64_t SignExtend : 16;									// 
			} Page4Kb;																				// PDE.PS == 0
			struct
			{
				std::uint64_t PageOffset : 21;									// byte offset into the physical page
				std::uint64_t PageDirectoryOffset : 9;					// index into the 512-entry page-directory table
				std::uint64_t PageDirectoryPointerOffset : 9;		// index into the 512-entry page-directory pointer table
				std::uint64_t PageMapLevel4Offset : 9;					// index into the 512-entry page-map level-4 table
				std::uint64_t SignExtend : 16;									// 
			} Page2Mb;																				// PDE.PS == 1
		} NonPageSize;																			// PDPE.PS == 0
		union
		{
			struct
			{
				std::uint64_t PageOffset : 30;									// byte offset into the physical page
				std::uint64_t PageDirectoryPointerOffset : 9;		// index into the 512-entry page-directory pointer table
				std::uint64_t PageMapLevel4Offset : 9;					// index into the 512-entry page-map level-4 table
				std::uint64_t SignExtend : 16;									// 
			} Page1Gb;																				// 
		} PageSize;																					// PDPE.PS == 1
	} x64;																								// x64 architecture
};

union PML4E
{
	union
	{
		std::uint64_t Value = 0;							// 
		struct
		{
			std::uint64_t P : 1;								// present
			std::uint64_t RW : 1;								// read / write
			std::uint64_t US : 1;								// user / supervisor
			std::uint64_t PWT : 1;							// page-level write through
			std::uint64_t PCD : 1;							// page-level cache disable
			std::uint64_t A : 1;								// accessed
			std::uint64_t Reserved0 : 1;				// 
			std::uint64_t Reserved1 : 2;				// 
			std::uint64_t AVL : 3;							// available to software
			std::uint64_t PDP : 40;							// 
			std::uint64_t Available : 11;				// 
			std::uint64_t NX : 1;								// no execute
		} Generic, Page4Kb, Page2Mb, Page1Gb;	// same for the 4kb, 2mb and 1gb page size
	} x64;																	// x64 architecture
};

union PDPE
{
	union
	{
		std::uint64_t Value = 0;									// 
		struct
		{
			std::uint64_t P : 1;										// present
			std::uint64_t RW : 1;										// read / write
			std::uint64_t US : 1;										// user / supervisor
			std::uint64_t PWT : 1;									// page-level write through
			std::uint64_t PCD : 1;									// page-level cache disable
			std::uint64_t A : 1;										// accessed
			std::uint64_t Reserved0 : 1;						// 
			std::uint64_t PS : 1;										// page size == 0
			std::uint64_t Reserved1 : 1;						// 
			std::uint64_t AVL : 3;									// available to software
			std::uint64_t Reserved2 : 51;						// 
			std::uint64_t NX : 1;										// no execute
		} Generic;																// 
		union
		{
			struct
			{
				std::uint64_t P : 1;									// present
				std::uint64_t RW : 1;									// read / write
				std::uint64_t US : 1;									// user / supervisor
				std::uint64_t PWT : 1;								// page-level write through
				std::uint64_t PCD : 1;								// page-level cache disable
				std::uint64_t A : 1;									// accessed
				std::uint64_t Reserved0 : 1;					// 
				std::uint64_t PS : 1;									// page size == 0
				std::uint64_t Reserved1 : 1;					// 
				std::uint64_t AVL : 3;								// available to software
				std::uint64_t PD : 40;								// 
				std::uint64_t Available : 11;					// 
				std::uint64_t NX : 1;									// no execute
			} Generic, Page4Kb, Page2Mb;						// same for the 4kb and 2mb page size
		} NonPageSize;														// PDPE.PS == 0
		union
		{
			struct
			{
				std::uint64_t P : 1;									// present
				std::uint64_t RW : 1;									// read / write
				std::uint64_t US : 1;									// user / supervisor
				std::uint64_t PWT : 1;								// page-level write through
				std::uint64_t PCD : 1;								// page-level cache disable
				std::uint64_t A : 1;									// accessed
				std::uint64_t D : 1;									// dirty
				std::uint64_t PS : 1;									// page size == 1
				std::uint64_t G : 1;									// global page
				std::uint64_t AVL : 3;								// available to software
				std::uint64_t PAT : 1;								// page-attribute table
				std::uint64_t Reserved0 : 17;					// 
				std::uint64_t PhysicalPageBase : 22;	// 
				std::uint64_t Available : 11;					// 
				std::uint64_t NX : 1;									// no execute
			} Page1Gb;															// 
		} PageSize;																// PDPE.PS == 1
	} x64;																			// x64 architecture
};

union PDE
{
	union
	{
		std::uint64_t Value = 0;								// 
		struct
		{
			std::uint64_t P : 1;									// present
			std::uint64_t RW : 1;									// read / write
			std::uint64_t US : 1;									// user / supervisor
			std::uint64_t PWT : 1;								// page-level write through
			std::uint64_t PCD : 1;								// page-level cache disable
			std::uint64_t A : 1;									// accessed
			std::uint64_t Reserved0 : 1;					// 
			std::uint64_t PS : 1;									// page size == 0
			std::uint64_t Reserved1 : 1;					// 
			std::uint64_t AVL : 3;								// available to software
			std::uint64_t Reserved2 : 51;					// 
			std::uint64_t NX : 1;									// no execute
		} Generic;															// 
		struct
		{
			std::uint64_t P : 1;									// present
			std::uint64_t RW : 1;									// read / write
			std::uint64_t US : 1;									// user / supervisor
			std::uint64_t PWT : 1;								// page-level write through
			std::uint64_t PCD : 1;								// page-level cache disable
			std::uint64_t A : 1;									// accessed
			std::uint64_t Reserved0 : 1;					// 
			std::uint64_t PS : 1;									// page size == 0
			std::uint64_t Reserved1 : 1;					// 
			std::uint64_t AVL : 3;								// available to software
			std::uint64_t PT : 40;								// 
			std::uint64_t Available : 11;					// 
			std::uint64_t NX : 1;									// no execute
		} Page4Kb;															// 
		struct
		{
			std::uint64_t P : 1;									// present
			std::uint64_t RW : 1;									// read / write
			std::uint64_t US : 1;									// user / supervisor
			std::uint64_t PWT : 1;								// page-level write through
			std::uint64_t PCD : 1;								// page-level cache disable
			std::uint64_t A : 1;									// accessed
			std::uint64_t D : 1;									// dirty
			std::uint64_t PS : 1;									// page size == 1
			std::uint64_t G : 1;									// global page
			std::uint64_t AVL : 3;								// available to software
			std::uint64_t PAT : 1;								// page-attribute table
			std::uint64_t Reserved0 : 8;					// 
			std::uint64_t PhysicalPageBase : 31;	// 
			std::uint64_t Available : 11;					// 
			std::uint64_t NX : 1;									// no execute
		} Page2Mb;															// 
	} x64;																		// x64 architecture
};

union PTE
{
	union
	{
		std::uint64_t Value = 0;								// 
		struct
		{
			std::uint64_t P : 1;									// present
			std::uint64_t RW : 1;									// read / write
			std::uint64_t US : 1;									// user / supervisor
			std::uint64_t PWT : 1;								// page-level write through
			std::uint64_t PCD : 1;								// page-level cache disable
			std::uint64_t A : 1;									// accessed
			std::uint64_t D : 1;									// dirty
			std::uint64_t PAT : 1;								// page-attribute table
			std::uint64_t G : 1;									// global page
			std::uint64_t AVL : 3;								// available to software
			std::uint64_t PhysicalPageBase : 40;	// 
			std::uint64_t Available : 11;					// 
			std::uint64_t NX : 1;									// no execute
		} Page4Kb;															// 
	} x64;																		// x64 architecture
};

union CR3
{
	std::uint64_t Value = 0;									// 
	union
	{
		std::uint64_t Value = 0;								// 
		struct
		{
			std::uint64_t Reserved0 : 3;					// 
			std::uint64_t PWT : 1;								// page-level write through
			std::uint64_t PCD : 1;								// page-level cache disable
			std::uint64_t Reserved1 : 7;					// 
			std::uint64_t PML4 : 40;							// page-map level-4 table base address
			std::uint64_t Reserved2 : 12;					// 
		} Bitmap;																// 
	} x64;																		// x64 architecture
};

struct PageData
{
	enum class Type
	{
		PageUnknown,
		PageSize4Kb,
		PageSize2Mb,
		PageSize1Gb,
	};

	Type m_type = Type::PageUnknown;		// page type
	PML4E* m_pml4e = nullptr;						// page-map level-4 entry
	PDPE* m_pdpe = nullptr;							// page-directory page entry
	PDE* m_pde = nullptr;								// page-directory entry
	PTE* m_pte = nullptr;								// page-table entry
};

// 
// restore structure packing
// 
#pragma pack( pop )

bool GetPageData( const void* pointer, PageData* page_data );
bool GetPageData( std::uintptr_t address, PageData* page_data );

bool IsPagePresent( const void* pointer, std::size_t* page_size );
bool IsPagePresent( std::uintptr_t address, std::size_t* page_size );

bool IsProcessPagePresent( PEPROCESS process, const void* pointer, std::size_t* page_size );
bool IsProcessPagePresent( PEPROCESS process, std::uintptr_t address, std::size_t* page_size );

bool IsMemoryRangePresent( PEPROCESS process, const void* pointer, std::size_t size );
bool IsMemoryRangePresent( PEPROCESS process, std::uintptr_t address, std::size_t size );

// 
// page operations
// 

bool IsPageReadable( std::uintptr_t address );
bool IsPageWriteable( std::uintptr_t address );
bool IsPageExecuteable( std::uintptr_t address );

} // namespace atom::win32