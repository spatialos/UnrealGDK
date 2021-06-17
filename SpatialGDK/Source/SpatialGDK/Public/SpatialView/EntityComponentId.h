// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/TypeHash.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct EntityComponentId
{
	EntityComponentId(FSpatialEntityId InEntityId, Worker_ComponentId InComponentId)
		: EntityId(InEntityId)
		, ComponentId(InComponentId)
	{
	}

	FSpatialEntityId EntityId;
	Worker_ComponentId ComponentId;

	friend bool operator==(const EntityComponentId& Lhs, const EntityComponentId& Rhs)
	{
		return Lhs.EntityId == Rhs.EntityId && Lhs.ComponentId == Rhs.ComponentId;
	}

	friend uint32 GetTypeHash(EntityComponentId Value)
	{
		return HashCombine(GetTypeHash(Value.EntityId), ::GetTypeHash(static_cast<uint32>(Value.ComponentId)));
	}
};

} // namespace SpatialGDK
