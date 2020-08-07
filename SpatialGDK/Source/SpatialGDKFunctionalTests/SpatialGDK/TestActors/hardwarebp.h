// Copyright (c) 2000 Mike Morearty <mike@morearty.com>
// Original source and docs: http://www.morearty.com/code/breakpoint

#ifndef _HARDWAREBP_H_
#define _HARDWAREBP_H_
#include "CoreMinimal.h"

class HardwareBreakpoint
{
public:
	HardwareBreakpoint() { m_index = -1; }
	~HardwareBreakpoint() { Clear(); }

	// The enum values correspond to the values used by the Intel Pentium,
	// so don't change them!
	enum Condition { Write = 1, Read /* or write! */ = 3 };

	void Set(void* address, int len /* 1, 2, or 4 */, Condition when);
	void Clear();

protected:

	inline void SetBits(uint64& dw, uint64 lowBit, uint64 bits, uint64 newValue)
	{
		uint64 mask = (1ull << bits) - 1; // e.g. 1 becomes 0001, 2 becomes 0011, 3 becomes 0111

		dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
	}

	int m_index; // -1 means not set; 0-3 means we've set that hardware bp
};


#endif // _HARDWAREBP_H_
