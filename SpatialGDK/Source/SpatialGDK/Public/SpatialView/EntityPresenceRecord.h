// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommonTypes.h"

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
	void AddEntity(FEntityId EntityId);
	void RemoveEntity(FEntityId EntityId);

	void Clear();

	const TArray<FEntityId>& GetEntitiesAdded() const;
	const TArray<FEntityId>& GetEntitiesRemoved() const;

private:
	TArray<FEntityId> EntitiesAdded;
	TArray<FEntityId> EntitiesRemoved;
};

} // namespace SpatialGDK
