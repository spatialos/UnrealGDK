#pragma once

struct FComponentIdGenerator
{
	FComponentIdGenerator(int StartId) : InitialId(StartId), NumIds(0)
	{
	}

	int GetNextAvailableId()
	{
		return InitialId + (NumIds++);
	}

	int GetNumUsedIds()
	{
		return NumIds;
	}

private:
	int InitialId;
	int NumIds;
};
