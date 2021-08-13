#pragma once

#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
struct FNeighborEntry
{
	static constexpr uint32 s_MaxNumNeighbours = 50;
	Worker_EntityId EntityId;
	int32 NumNeighbours = 0;
	Worker_EntityId Neighbours[s_MaxNumNeighbours];
	float SquaredDist[s_MaxNumNeighbours];
};

class FEntitySpatialScene
{
public:
	FEntitySpatialScene();
	~FEntitySpatialScene();

	void Add(Worker_EntityId, FVector, bool bUsesNN);
	void Update(Worker_EntityId, FVector);
	void Remove(Worker_EntityId);

	bool UpdateNeighbours(float SearchRadius);

	const TArray<FNeighborEntry>& GetNeighbourInfo() { return NeighboursInfo; }

private:
	struct Impl;
	TUniquePtr<Impl> m_Impl;

	TMap<Worker_EntityId, int32> EntityToSlot;
	TArray<FNeighborEntry> NeighboursInfo;
};
} // namespace SpatialGDK
