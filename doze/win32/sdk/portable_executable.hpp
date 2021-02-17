#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

#define IMAGE_DOS_SIGNATURE						0x5A4D
#define IMAGE_NT_SIGNATURE						0x00004550

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC			0x010B
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC			0x020B

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES		16

#define IMAGE_DIRECTORY_ENTRY_EXPORT			0
#define IMAGE_DIRECTORY_ENTRY_IMPORT			1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE			2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION			3
#define IMAGE_DIRECTORY_ENTRY_SECURITY			4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC			5
#define IMAGE_DIRECTORY_ENTRY_DEBUG				6
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE		7
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR			8
#define IMAGE_DIRECTORY_ENTRY_TLS				9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG		10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT		11
#define IMAGE_DIRECTORY_ENTRY_IAT				12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT		13
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR	14

#define IMAGE_REL_BASED_ABSOLUTE				0
#define IMAGE_REL_BASED_HIGH					1
#define IMAGE_REL_BASED_LOW						2
#define IMAGE_REL_BASED_HIGHLOW					3
#define IMAGE_REL_BASED_HIGHADJ					4
#define IMAGE_REL_BASED_MIPS_JMPADDR			5
#define IMAGE_REL_BASED_SECTION					6
#define IMAGE_REL_BASED_REL32					7
#define IMAGE_REL_BASED_MIPS_JMPADDR16			9
#define IMAGE_REL_BASED_IA64_IMM64				9
#define IMAGE_REL_BASED_DIR64					10

#define IMAGE_SIZEOF_BASE_RELOCATION			8

#define IMAGE_FILE_RELOCS_STRIPPED				0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE				0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED			0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED			0x0008
#define IMAGE_FILE_AGGRESIVE_WS_TRIM			0x0010
#define IMAGE_FILE_LARGE_ADDRESS_AWARE			0x0020
#define IMAGE_FILE_BYTES_REVERSED_LO			0x0080
#define IMAGE_FILE_32BIT_MACHINE				0x0100
#define IMAGE_FILE_DEBUG_STRIPPED				0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP		0x0400
#define IMAGE_FILE_NET_RUN_FROM_SWAP			0x0800
#define IMAGE_FILE_SYSTEM						0x1000
#define IMAGE_FILE_DLL							0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY				0x4000
#define IMAGE_FILE_BYTES_REVERSED_HI			0x8000

#define IMAGE_ORDINAL_FLAG32					0x80000000
#define IMAGE_ORDINAL_FLAG64					0x8000000000000000

#define IMAGE_ORDINAL32( Ordinal )				( Ordinal & 0xFFFF )
#define IMAGE_ORDINAL64( Ordinal )				( Ordinal & 0xFFFF )

#define IMAGE_SNAP_BY_ORDINAL32( Ordinal )		( ( Ordinal & IMAGE_ORDINAL_FLAG32 ) != 0 )
#define IMAGE_SNAP_BY_ORDINAL64( Ordinal )		( ( Ordinal & IMAGE_ORDINAL_FLAG64 ) != 0 )

#define IMAGE_SIZEOF_SECTION_HEADER          40

//
// Section characteristics.
//
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
#define IMAGE_SCN_GPREL                      0x00008000  // Section content can be accessed relative to GP
#define IMAGE_SCN_MEM_FARDATA                0x00008000
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000  //
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000  //
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000  //
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000  //
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000  //
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000  //
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000  //
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000  //
// Unused                                    0x00F00000
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// TLS Characteristic Flags
//
#define IMAGE_SCN_SCALE_INDEX                0x00000001  // Tls index is scaled

namespace atom::win32
{ }