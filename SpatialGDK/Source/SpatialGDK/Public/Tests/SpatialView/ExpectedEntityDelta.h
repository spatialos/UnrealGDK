// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialView/ComponentData.h"
#include "SpatialView/EntityDelta.h"

namespace SpatialGDK
{
struct ExpectedEntityDelta
{
	Worker_EntityId EntityId;
	enum
	{
		UPDATE,
		ADD,
		REMOVE,
		TEMPORARILY_REMOVED
	} Type;
	TArray<ComponentData> DataStorage;
	TArray<ComponentUpdate> UpdateStorage;
	TArray<ComponentChange> ComponentsAdded;
	TArray<ComponentChange> ComponentsRemoved;
	TArray<ComponentChange> ComponentUpdates;
	TArray<ComponentChange> ComponentsRefreshed;
	TArray<AuthorityChange> AuthorityGained;
	TArray<AuthorityChange> AuthorityLost;
	TArray<AuthorityChange> AuthorityLostTemporarily;
};
} // namespace SpatialGDK
