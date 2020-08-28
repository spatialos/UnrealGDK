// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialView/EntityDelta.h"

namespace SpatialGDK
{
struct ExpectedEntityDelta
{
	Worker_EntityId EntityId;
	bool bAdded;
	bool bRemoved;
	TArray<ComponentChange> ComponentsAdded;
	TArray<ComponentChange> ComponentsRemoved;
	TArray<ComponentChange> ComponentUpdates;
	TArray<ComponentChange> ComponentsRefreshed;
	TArray<AuthorityChange> AuthorityGained;
	TArray<AuthorityChange> AuthorityLost;
	TArray<AuthorityChange> AuthorityLostTemporarily;
};
} // namespace SpatialGDK
