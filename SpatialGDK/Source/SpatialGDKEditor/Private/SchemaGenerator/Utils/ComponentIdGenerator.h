// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FComponentIdGenerator
{
	const uint32 RESERVED_COMPONENT_ID_START = 19000;
	const uint32 RESERVED_COMPONENT_ID_END = 19999;

	FComponentIdGenerator(uint32 InNextId)
	{
		if (RESERVED_COMPONENT_ID_START <= InNextId && InNextId <= RESERVED_COMPONENT_ID_END)
		{
			InNextId = RESERVED_COMPONENT_ID_END + 1;
		}
		else
		{
			NextId = InNextId;
		}
	}

	uint32 Next()
	{
		uint32 Result = NextId;
		NextId++;
		if (RESERVED_COMPONENT_ID_START <= NextId && NextId <= RESERVED_COMPONENT_ID_END)
		{
			NextId = RESERVED_COMPONENT_ID_END + 1;
		}
		return Result;
	}

	uint32 Peek() const
	{
		return NextId;
	}

private:
	uint32 NextId;
};
