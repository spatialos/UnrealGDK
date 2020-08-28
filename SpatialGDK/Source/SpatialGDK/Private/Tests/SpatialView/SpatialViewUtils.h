// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/ViewDelta.h"
#include "SpatialView/WorkerView.h"

namespace SpatialGDK
{
enum EntityStatus
{
	ADD,
	REMOVE,
	UPDATE
};

inline void AddComponentAddedChange(TArray<ComponentChange>& Changes, int ComponentId, double Value)
{
	const auto Data = Schema_CreateComponentData();
	Schema_Object* Fields = Schema_GetComponentDataFields(Data);
	Schema_AddDouble(Fields, ComponentId, Value);
	Changes.Push(ComponentChange(ComponentId, Schema_CopyComponentData(Data)));
}

inline void AddComponentUpdateChange(TArray<ComponentChange>& Changes, int ComponentId, double Value)
{
	const auto Update = Schema_CreateComponentUpdate();
	Schema_Object* Fields = Schema_GetComponentUpdateFields(Update);
	Schema_AddDouble(Fields, ComponentId, Value);
	Changes.Push(ComponentChange(ComponentId, Schema_CopyComponentUpdate(Update)));
}

inline void AddComponentRemovedChange(TArray<ComponentChange>& Changes, int ComponentId)
{
	Changes.Push(ComponentChange(ComponentId));
}

inline EntityDelta CreateEntityDelta(int EntityId, EntityStatus Status)
{
	EntityDelta Delta;
	Delta.EntityId = EntityId;
	Delta.bAdded = Status == ADD;
	Delta.bRemoved = Status == REMOVE;
	Delta.AuthorityGained = ComponentSpan<AuthorityChange>();
	Delta.AuthorityLost = ComponentSpan<AuthorityChange>();
	Delta.AuthorityLostTemporarily = ComponentSpan<AuthorityChange>();
	Delta.ComponentsAdded = ComponentSpan<ComponentChange>();
	Delta.ComponentsRemoved = ComponentSpan<ComponentChange>();
	Delta.ComponentsRefreshed = ComponentSpan<ComponentChange>();
	Delta.ComponentUpdates = ComponentSpan<ComponentChange>();
	return Delta;
}

inline void AddEntityToView(EntityView& View, int EntityId)
{
	View.Add(EntityId, { {}, {} });
}

inline void AddComponentToView(EntityView& View, int EntityId, int ComponentId, double Value)
{
	ComponentData TestComponentData = CreateTestComponentData(ComponentId, Value);
	View[EntityId].Components.Push(MoveTemp(TestComponentData));
}

inline void AddAuthorityToView(EntityView& View, int EntityId, int ComponentId)
{
	View[EntityId].Authority.Push(ComponentId);
}

inline bool AreEquivalent(EntityView& Lhs, EntityView& Rhs)
{
	TArray<Worker_EntityId_Key> LhsKeys;
	TArray<Worker_EntityId_Key> RhsKeys;
	Lhs.GetKeys(LhsKeys);
	Rhs.GetKeys(RhsKeys);
	if (!AreEquivalent(LhsKeys, RhsKeys, CompareWorkerEntityIdKey))
	{
		return false;
	}

	for (const auto& Pair : Lhs)
	{
		const long EntityId = Pair.Key;
		const EntityViewElement* LhsElement = &Pair.Value;
		const EntityViewElement* RhsElement = &Rhs[EntityId];
		if (!AreEquivalent(LhsElement->Components, RhsElement->Components))
		{
			return false;
		}

		if (!AreEquivalent(LhsElement->Authority, RhsElement->Authority, CompareWorkerComponentId))
		{
			return false;
		}
	}

	return true;
}

inline bool AreEquivalent(ViewDelta& Lhs, ViewDelta& Rhs)
{
	// This does not check for events or commands atm
	TArray<EntityDelta> LhsEntityDeltas = Lhs.GetEntityDeltas();
	TArray<EntityDelta> RhsEntityDeltas = Rhs.GetEntityDeltas();
	if (LhsEntityDeltas.Num() != RhsEntityDeltas.Num())
	{
		return false;
	}

	for (int i = 0; i < LhsEntityDeltas.Num(); i++)
	{
		EntityDelta LhsEntityDelta = LhsEntityDeltas[i];
		EntityDelta RhsEntityDelta = RhsEntityDeltas[i];
		if (LhsEntityDelta.EntityId != RhsEntityDelta.EntityId)
		{
			return false;
		}

		if (LhsEntityDelta.bAdded != RhsEntityDelta.bAdded)
		{
			return false;
		}

		if (LhsEntityDelta.bRemoved != RhsEntityDelta.bRemoved)
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.AuthorityGained, RhsEntityDelta.AuthorityGained, CompareAuthorityChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.AuthorityLost, RhsEntityDelta.AuthorityLost, CompareAuthorityChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.AuthorityLostTemporarily, RhsEntityDelta.AuthorityLostTemporarily, CompareAuthorityChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.ComponentsAdded, RhsEntityDelta.ComponentsAdded, CompareComponentChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.ComponentsRemoved, RhsEntityDelta.ComponentsRemoved, CompareComponentChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.ComponentsRefreshed, RhsEntityDelta.ComponentsRefreshed, CompareComponentChanges))
		{
			return false;
		}

		if (!AreEquivalent(LhsEntityDelta.ComponentUpdates, RhsEntityDelta.ComponentUpdates, CompareComponentChanges))
		{
			return false;
		}
	}

	return true;
}
} // namespace SpatialGDK
