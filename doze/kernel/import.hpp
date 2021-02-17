#pragma once

#include "../include/auto.hpp"
#include "../include/base.hpp"
#include "../include/win32.hpp"

#include "../win32/sdk/image_nt_headers.hpp"
#include "../win32/sdk/system_information_class.hpp"

extern "C"
{

	NTKERNELAPI PVOID NTAPI PsGetProcessPeb( IN PEPROCESS Process );
	NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process( IN PEPROCESS Process );
	NTKERNELAPI PVOID NTAPI PsGetProcessSectionBaseAddress( IN PEPROCESS Process );

	NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory( IN PEPROCESS FromProcess,
																									IN PVOID FromAddress,
																									IN PEPROCESS ToProcess,
																									OUT PVOID ToAddress,
																									IN SIZE_T BufferSize,
																									IN KPROCESSOR_MODE PreviousMode,
																									OUT PSIZE_T NumberOfBytesCopied );

	NTKERNELAPI atom::win32::IMAGE_NT_HEADERS* NTAPI RtlImageNtHeader( PVOID Base );

	NTKERNELAPI PVOID NTAPI RtlImageDirectoryEntryToData( PVOID ImageBase,
																												BOOLEAN MappedAsImage,
																												USHORT DirectoryEntry,
																												PULONG Size );

	NTKERNELAPI NTSTATUS NTAPI ZwQuerySystemInformation( atom::win32::SYSTEM_INFORMATION_CLASS SystemInformationClass,
																											 PVOID SystemInformation,
																											 ULONG SystemInformationLength,
																											 PULONG ReturnLength );

}; // extern "C"