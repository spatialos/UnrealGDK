#pragma once

#include "SpatialView/CommonTypes.h"

#include "Templates/TypeHash.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct EntityComponentId
{
	EntityComponentId(FEntityId InEntityId, FComponentId InComponentId)
		: EntityId(InEntityId)
		, ComponentId(InComponentId)
	{
	}

	FEntityId EntityId;
	FComponentId ComponentId;

	friend bool operator==(const EntityComponentId& Lhs, const EntityComponentId& Rhs)
	{
		return Lhs.EntityId == Rhs.EntityId && Lhs.ComponentId == Rhs.ComponentId;
	}

	friend uint32 GetTypeHash(EntityComponentId Value)
	{
		return HashCombine(::GetTypeHash(static_cast<int64>(Value.EntityId)), ::GetTypeHash(static_cast<uint32>(Value.ComponentId)));
	}
};

} // namespace SpatialGDK
