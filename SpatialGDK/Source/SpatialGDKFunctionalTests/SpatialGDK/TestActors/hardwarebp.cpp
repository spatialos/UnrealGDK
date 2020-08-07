// Copyright (c) 2000 Mike Morearty <mike@morearty.com>
// Original source and docs: http://www.morearty.com/code/breakpoint

#include "hardwarebp.h"
#include "Windows/WindowsSystemIncludes.h"
//#include "processthreadsapi.h"

#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <psapi.h>
#include "Windows/HideWindowsPlatformTypes.h"

void HardwareBreakpoint::Set(void* address, int len, Condition when)
{
	// make sure this breakpoint isn't already set
	check(m_index == -1);

	CONTEXT cxt;
	HANDLE thisThread = GetCurrentThread();

	switch (len)
	{
	case 1: len = 0; break;
	case 2: len = 1; break;
	case 4: len = 3; break;
	default: check(false); // invalid length
	}

	// The only registers we care about are the debug registers
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// Read the register values
	if (!GetThreadContext(thisThread, &cxt))
		check(false);

	// Find an available hardware register
	for (m_index = 0; m_index < 4; ++m_index)
	{
		if ((cxt.Dr7 & (1ull << (m_index*2))) == 0)
			break;
	}
	check(m_index < 4); // All hardware breakpoint registers are already being used

	switch (m_index)
	{
	case 0: cxt.Dr0 = (DWORD64) address; break;
	case 1: cxt.Dr1 = (DWORD64) address; break;
	case 2: cxt.Dr2 = (DWORD64) address; break;
	case 3: cxt.Dr3 = (DWORD64) address; break;
	default: check(false); // m_index has bogus value
	}

	SetBits(cxt.Dr7, 16 + (m_index*4), 2, when);
	SetBits(cxt.Dr7, 18 + (m_index*4), 2, len);
	SetBits(cxt.Dr7, m_index*2,        1, 1);

	// Write out the new debug registers
	if (!SetThreadContext(thisThread, &cxt))
		check(false);
}


void HardwareBreakpoint::Clear()
{
	if (m_index != -1)
	{
		CONTEXT cxt;
		HANDLE thisThread = GetCurrentThread();

		// The only registers we care about are the debug registers
		cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;

		// Read the register values
		if (!GetThreadContext(thisThread, &cxt))
			check(false);

		// Zero out the debug register settings for this breakpoint
		check(m_index >= 0 && m_index < 4); // m_index has bogus value
		SetBits(cxt.Dr7, m_index*2, 1, 0);

		// Write out the new debug registers
		if (!SetThreadContext(thisThread, &cxt))
			check(false);

		m_index = -1;
	}
}

