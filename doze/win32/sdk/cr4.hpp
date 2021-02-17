#pragma once

#include "../../include/auto.hpp"
#include "../../include/base.hpp"
#include "../../include/win32.hpp"

namespace atom::win32
{

union CR4
{
	unsigned long long Value;
	union
	{
		unsigned int Value;
		struct
		{
			unsigned int VME : 1; // Virtual 8086-Mode Extensions
			unsigned int PVI : 1; // Protected-Mode Virtual Interrupts
			unsigned int TSD : 1; // Time-Stamp Disable
			unsigned int DE : 1; // Debugging Extensions
			unsigned int PSE : 1; // Page Size Extensions
			unsigned int PAE : 1; // Physical Address Extension
			unsigned int MCE : 1; // Machine Check Enable
			unsigned int PGE : 1; // Page Global Enable
			unsigned int PCE : 1; // Performance-Monitoring Counter Enable
			unsigned int OSFXSR : 1; // Operating-System FXSAVE/FXRSTOR Support
			unsigned int OSXMMEXCPT : 1; // Operating System Unmasked Exception Support
			unsigned int Reserved0 : 5;
			unsigned int FSGSBASE : 1; // Enable RDFSBASE, RDGSBASE, WRFSBASE, WRGSBASE instructions
			unsigned int Reserved1 : 1;
			unsigned int OSXSAVE : 1; // XSAVE and Processor Extended States Enable Bit
			unsigned int Reserved2 : 13;
		} Bitmap;
	} x32;
	union
	{
		unsigned long long Value;
		struct
		{
			unsigned long long VME : 1; // Virtual 8086-Mode Extensions
			unsigned long long PVI : 1; // Protected-Mode Virtual Interrupts
			unsigned long long TSD : 1; // Time-Stamp Disable
			unsigned long long DE : 1; // Debugging Extensions
			unsigned long long PSE : 1; // Page Size Extensions
			unsigned long long PAE : 1; // Physical Address Extension
			unsigned long long MCE : 1; // Machine Check Enable
			unsigned long long PGE : 1; // Page Global Enable
			unsigned long long PCE : 1; // Performance-Monitoring Counter Enable
			unsigned long long OSFXSR : 1; // Operating-System FXSAVE/FXRSTOR Support
			unsigned long long OSXMMEXCPT : 1; // Operating System Unmasked Exception Support
			unsigned long long Reserved0 : 5;
			unsigned long long FSGSBASE : 1; // Enable RDFSBASE, RDGSBASE, WRFSBASE, WRGSBASE instructions
			unsigned long long Reserved1 : 1;
			unsigned long long OSXSAVE : 1; // XSAVE and Processor Extended States Enable Bit
			unsigned long long Reserved2 : 45;
		} Bitmap;
	} x64;
};

}