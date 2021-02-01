// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/EntityComponentId.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
class EntityComponentUpdateRecord
{
public:
	EntityComponentUpdateRecord() = default;
	~EntityComponentUpdateRecord() = default;

	// Moveable, not copyable.
	EntityComponentUpdateRecord(const EntityComponentUpdateRecord&) = delete;
	EntityComponentUpdateRecord(EntityComponentUpdateRecord&&) = default;
	EntityComponentUpdateRecord& operator=(const EntityComponentUpdateRecord&) = delete;
	EntityComponentUpdateRecord& operator=(EntityComponentUpdateRecord&&) = default;

	// Record a complete update for an entity-component.
	// The entity-component will be recorded as completely-updated.
	void AddComponentDataAsUpdate(Worker_EntityId EntityId, ComponentData CompleteUpdate);

	// Record a update for an entity-component.
	// If there is no record for the entity-component it will be recorded as updated.
	void AddComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update);

	// Clear all records for an entity-component.
	void RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// Clear all records.
	void Clear();

	// Get all entity-components recorded as updated and their associated data.
	const TArray<EntityComponentUpdate>& GetUpdates() const;
	// Get all entity-components recorded as completely-updated and their associated data.
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

private:
	void InsertOrMergeUpdate(Worker_EntityId EntityId, ComponentUpdate Update);
	void InsertOrSetCompleteUpdate(Worker_EntityId EntityId, ComponentData CompleteUpdate);

	TArray<EntityComponentUpdate> Updates;
	TArray<EntityComponentCompleteUpdate> CompleteUpdates;
};

} // namespace SpatialGDK
