// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
// An entity can be recorded as at most one of
//	added
//	removed
class EntityPresenceRecord
{
public:
	void AddEntity(Worker_EntityId EntityId);
	void RemoveEntity(Worker_EntityId EntityId);

	void Clear();

	const TArray<Worker_EntityId>& GetEntitiesAdded() const;
	const TArray<Worker_EntityId>& GetEntitiesRemoved() const;

private:
	TArray<Worker_EntityId> EntitiesAdded;
	TArray<Worker_EntityId> EntitiesRemoved;
};

} // namespace SpatialGDK
