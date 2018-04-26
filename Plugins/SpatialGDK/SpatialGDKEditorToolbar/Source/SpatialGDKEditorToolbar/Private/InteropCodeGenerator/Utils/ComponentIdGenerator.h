// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FComponentIdGenerator
{
	FComponentIdGenerator(int StartId)
	: InitialId(StartId), NumIds(0)
	{
	}

	int GetNextAvailableId()
	{
		return InitialId + (NumIds++);
	}

	int GetNumUsedIds() const
	{
		return NumIds;
	}

  private:
	int InitialId;
	int NumIds;
};
