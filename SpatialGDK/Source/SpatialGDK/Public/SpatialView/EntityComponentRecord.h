// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/EntityComponentId.h"
#include "SpatialView/EntityComponentUpdateRecord.h"

namespace SpatialGDK
{
// Can be recorded as at most one of
//	added
//	removed
//	updated
//	Corollary of this is that if the component is recorded as added, that events received in the same tick will be dropped.
class EntityComponentRecord
{
public:
	void AddComponent(FEntityId EntityId, ComponentData Data);
	void RemoveComponent(FEntityId EntityId, Worker_ComponentId ComponentId);
	void AddComponentAsUpdate(FEntityId EntityId, ComponentData Data);
	void AddUpdate(FEntityId EntityId, ComponentUpdate Update);

	void Clear();

	const TArray<EntityComponentData>& GetComponentsAdded() const;
	const TArray<EntityComponentId>& GetComponentsRemoved() const;

	const TArray<EntityComponentUpdate>& GetUpdates() const;
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

private:
	TArray<EntityComponentData> ComponentsAdded;
	TArray<EntityComponentId> ComponentsRemoved;
	EntityComponentUpdateRecord UpdateRecord;
};

} // namespace SpatialGDK
