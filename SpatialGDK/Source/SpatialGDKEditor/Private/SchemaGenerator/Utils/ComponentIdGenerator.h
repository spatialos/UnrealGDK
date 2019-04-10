// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FComponentIdGenerator
{
	FComponentIdGenerator(uint32 NextId) : NextId(NextId)
	{
	}

	uint32 GetExistingOrNext(const uint32 InComponentId = SpatialConstants::INVALID_COMPONENT_ID)
	{
		if (InComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			return InComponentId;
		}
		return Next();
	}

	uint32 Next()
	{
		return NextId++;
	}

	uint32 Peek()
	{
		return NextId;
	}

private:
	uint32 NextId;
};
