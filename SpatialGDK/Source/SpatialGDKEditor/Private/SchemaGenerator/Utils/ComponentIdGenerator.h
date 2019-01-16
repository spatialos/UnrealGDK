// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FComponentIdGenerator
{
	FComponentIdGenerator(int StartId) : InitialId(StartId), IdReturned(StartId), NumIds(0)
	{
	}

	int GetNextAvailableId(const uint32 InComponentId)
	{
		IdReturned = InComponentId != SpatialConstants::INVALID_COMPONENT_ID ? InComponentId : InitialId + (NumIds++);
		return IdReturned;
	}

	int GetCurrentId() const
	{
		return IdReturned;
	}

	int GetNumUsedIds() const
	{
		return NumIds;
	}

private:
	int InitialId;
	int NumIds;
	int IdReturned;
};
