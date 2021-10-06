// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/EntityComponentRecord.h"

namespace SpatialGDK
{
void EntityComponentRecord::AddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	const EntityComponentId Id = { EntityId, Data.GetComponentId() };

	// If the component is recorded as removed then transition to complete-updated.
	// otherwise record it as added.
	if (ComponentsRemoved.RemoveSingleSwap(Id))
	{
		UpdateRecord.AddComponentDataAsUpdate(EntityId, MoveTemp(Data));
	}
	else
	{
		ComponentsAdded.Push(EntityComponentData{ EntityId, MoveTemp(Data) });
	}
}

void EntityComponentRecord::RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	const EntityComponentId Id = { EntityId, ComponentId };
	const EntityComponentData* FoundComponentAdded = ComponentsAdded.FindByPredicate(EntityComponentIdEquality{ Id });

	// If the component is recorded as added then erase the record.
	// Otherwise record it as removed (additionally making sure it isn't recorded as updated).
	if (FoundComponentAdded)
	{
		ComponentsAdded.RemoveAtSwap(FoundComponentAdded - ComponentsAdded.GetData());
	}
	else
	{
		UpdateRecord.RemoveComponent(EntityId, ComponentId);
		ComponentsRemoved.Push(Id);
	}
}

void EntityComponentRecord::AddComponentAsUpdate(Worker_EntityId EntityId, ComponentData Data)
{
	const EntityComponentId Id = { EntityId, Data.GetComponentId() };
	EntityComponentData* FoundComponentAdded = ComponentsAdded.FindByPredicate(EntityComponentIdEquality{ Id });

	// If the entity-component is recorded is added, then merge the update to the added component.
	// Otherwise handle it as an update.
	if (FoundComponentAdded)
	{
		FoundComponentAdded->Data = MoveTemp(Data);
	}
	else
	{
		UpdateRecord.AddComponentDataAsUpdate(EntityId, MoveTemp(Data));
	}
}

void EntityComponentRecord::AddUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	const EntityComponentId Id = { EntityId, Update.GetComponentId() };
	EntityComponentData* FoundComponentAdded = ComponentsAdded.FindByPredicate(EntityComponentIdEquality{ Id });

	// If the entity-component is recorded is added, then merge the update to the added component.
	// Otherwise handle it as an update.
	if (FoundComponentAdded)
	{
		FoundComponentAdded->Data.ApplyUpdate(Update);
	}
	else
	{
		UpdateRecord.AddComponentUpdate(EntityId, MoveTemp(Update));
	}
}

void EntityComponentRecord::Clear()
{
	ComponentsAdded.Empty();
	ComponentsRemoved.Empty();
	UpdateRecord.Clear();
}

const TArray<EntityComponentData>& EntityComponentRecord::GetComponentsAdded() const
{
	return ComponentsAdded;
}

const TArray<EntityComponentId>& EntityComponentRecord::GetComponentsRemoved() const
{
	return ComponentsRemoved;
}

const TArray<EntityComponentUpdate>& EntityComponentRecord::GetUpdates() const
{
	return UpdateRecord.GetUpdates();
}

const TArray<EntityComponentCompleteUpdate>& EntityComponentRecord::GetCompleteUpdates() const
{
	return UpdateRecord.GetCompleteUpdates();
}

} // namespace SpatialGDK
