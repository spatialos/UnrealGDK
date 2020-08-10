// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/EntityComponentUpdateRecord.h"

namespace SpatialGDK
{
void EntityComponentUpdateRecord::AddComponentDataAsUpdate(Worker_EntityId EntityId, ComponentData CompleteUpdate)
{
	const EntityComponentId Id = { EntityId, CompleteUpdate.GetComponentId() };
	EntityComponentUpdate* FoundUpdate = Updates.FindByPredicate(EntityComponentIdEquality{ Id });

	if (FoundUpdate)
	{
		CompleteUpdates.Emplace(EntityComponentCompleteUpdate{ EntityId, MoveTemp(CompleteUpdate), MoveTemp(FoundUpdate->Update) });
		Updates.RemoveAtSwap(FoundUpdate - Updates.GetData());
	}
	else
	{
		InsertOrSetCompleteUpdate(EntityId, MoveTemp(CompleteUpdate));
	}
}

void EntityComponentUpdateRecord::AddComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	const EntityComponentId Id = { EntityId, Update.GetComponentId() };
	EntityComponentCompleteUpdate* FoundCompleteUpdate = CompleteUpdates.FindByPredicate(EntityComponentIdEquality{ Id });

	if (FoundCompleteUpdate != nullptr)
	{
		FoundCompleteUpdate->CompleteUpdate.ApplyUpdate(Update);
		FoundCompleteUpdate->Events.Merge(MoveTemp(Update));
	}
	else
	{
		InsertOrMergeUpdate(EntityId, MoveTemp(Update));
	}
}

void EntityComponentUpdateRecord::RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	const EntityComponentId Id = { EntityId, ComponentId };

	const EntityComponentUpdate* FoundUpdate = Updates.FindByPredicate(EntityComponentIdEquality{ Id });
	if (FoundUpdate)
	{
		Updates.RemoveAtSwap(FoundUpdate - Updates.GetData());
	}
	// If the entity-component is recorded as updated, it can't also be completely-updated so we don't need to search for it.
	else
	{
		const EntityComponentCompleteUpdate* FoundCompleteUpdate = CompleteUpdates.FindByPredicate(EntityComponentIdEquality{ Id });
		if (FoundCompleteUpdate)
		{
			CompleteUpdates.RemoveAtSwap(FoundCompleteUpdate - CompleteUpdates.GetData());
		}
	}
}

void EntityComponentUpdateRecord::Clear()
{
	Updates.Empty();
	CompleteUpdates.Empty();
}

const TArray<EntityComponentUpdate>& EntityComponentUpdateRecord::GetUpdates() const
{
	return Updates;
}

const TArray<EntityComponentCompleteUpdate>& EntityComponentUpdateRecord::GetCompleteUpdates() const
{
	return CompleteUpdates;
}

void EntityComponentUpdateRecord::InsertOrMergeUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	const EntityComponentId Id = { EntityId, Update.GetComponentId() };
	EntityComponentUpdate* FoundUpdate = Updates.FindByPredicate(EntityComponentIdEquality{ Id });

	if (FoundUpdate != nullptr)
	{
		FoundUpdate->Update.Merge(MoveTemp(Update));
	}
	else
	{
		Updates.Emplace(EntityComponentUpdate{ EntityId, MoveTemp(Update) });
	}
}

void EntityComponentUpdateRecord::InsertOrSetCompleteUpdate(Worker_EntityId EntityId, ComponentData CompleteUpdate)
{
	const EntityComponentId Id = { EntityId, CompleteUpdate.GetComponentId() };
	EntityComponentCompleteUpdate* FoundCompleteUpdate = CompleteUpdates.FindByPredicate(EntityComponentIdEquality{ Id });

	if (FoundCompleteUpdate != nullptr)
	{
		FoundCompleteUpdate->CompleteUpdate = MoveTemp(CompleteUpdate);
	}
	else
	{
		CompleteUpdates.Emplace(EntityComponentCompleteUpdate{ EntityId, MoveTemp(CompleteUpdate), ComponentUpdate(Id.ComponentId) });
	}
}

} // namespace SpatialGDK
