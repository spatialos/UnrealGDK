// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FComponentIdGenerator
{
	const uint32 RESERVED_COMPONENT_ID_START = 19000;
	const uint32 RESERVED_COMPONENT_ID_END = 19999;

	FComponentIdGenerator(uint32 InNextId)
		: NextId(InNextId)
	{
		ValidateNextId();
	}

	uint32 Next()
	{
		uint32 Result = NextId++;
		ValidateNextId();
		return Result;
	}

	uint32 Peek() const { return NextId; }

private:
	void ValidateNextId()
	{
		if (RESERVED_COMPONENT_ID_START <= NextId && NextId <= RESERVED_COMPONENT_ID_END)
		{
			NextId = RESERVED_COMPONENT_ID_END + 1;
		}
	}

	uint32 NextId;
};
