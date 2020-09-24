#include "SpatialView/EntityPresenceRecord.h"

namespace SpatialGDK
{
void EntityPresenceRecord::AddEntity(Worker_EntityId EntityId)
{
	if (EntitiesRemoved.RemoveSingleSwap(EntityId) == 0)
	{
		EntitiesAdded.Add(EntityId);
	}
}

void EntityPresenceRecord::RemoveEntity(Worker_EntityId EntityId)
{
	if (EntitiesAdded.RemoveSingleSwap(EntityId) == 0)
	{
		EntitiesRemoved.Add(EntityId);
	}
}

void EntityPresenceRecord::Clear()
{
	EntitiesAdded.Empty();
	EntitiesRemoved.Empty();
}

const TArray<Worker_EntityId>& EntityPresenceRecord::GetEntitiesAdded() const
{
	return EntitiesAdded;
}

const TArray<Worker_EntityId>& EntityPresenceRecord::GetEntitiesRemoved() const
{
	return EntitiesRemoved;
}

} // namespace SpatialGDK
