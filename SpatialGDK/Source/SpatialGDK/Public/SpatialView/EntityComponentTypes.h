// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"
#include "SpatialView/EntityComponentId.h"

namespace SpatialGDK
{
struct EntityComponentUpdate
{
	Worker_EntityId EntityId;
	ComponentUpdate Update;

	EntityComponentId GetEntityComponentId() const { return { EntityId, Update.GetComponentId() }; }
};

struct EntityComponentData
{
	Worker_EntityId EntityId;
	ComponentData Data;

	EntityComponentId GetEntityComponentId() const { return { EntityId, Data.GetComponentId() }; }
};

struct EntityComponentCompleteUpdate
{
	Worker_EntityId EntityId;
	ComponentData CompleteUpdate;
	ComponentUpdate Events;

	EntityComponentId GetEntityComponentId() const { return { EntityId, CompleteUpdate.GetComponentId() }; }
};

struct EntityComponentIdEquality
{
	EntityComponentId Id;

	bool operator()(const EntityComponentUpdate& Element) const { return Element.GetEntityComponentId() == Id; }

	bool operator()(const EntityComponentData& Element) const { return Element.GetEntityComponentId() == Id; }

	bool operator()(const EntityComponentCompleteUpdate& Element) const { return Element.GetEntityComponentId() == Id; }
};

} // namespace SpatialGDK
