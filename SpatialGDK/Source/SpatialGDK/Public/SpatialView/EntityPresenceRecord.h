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
	void AddEntity(FSpatialEntityId EntityId);
	void RemoveEntity(FSpatialEntityId EntityId);

	void Clear();

	const TArray<FSpatialEntityId>& GetEntitiesAdded() const;
	const TArray<FSpatialEntityId>& GetEntitiesRemoved() const;

private:
	TArray<FSpatialEntityId> EntitiesAdded;
	TArray<FSpatialEntityId> EntitiesRemoved;
};

} // namespace SpatialGDK
