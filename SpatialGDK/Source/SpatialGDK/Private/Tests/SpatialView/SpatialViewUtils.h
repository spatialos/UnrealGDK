// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewDelta.h"
#include "SpatialView/WorkerView.h"

namespace SpatialGDK
{
inline void SetFromOpList(ViewDelta& Delta, EntityView& View, EntityComponentOpListBuilder OpListBuilder)
{
	OpList Ops = MoveTemp(OpListBuilder).CreateOpList();
	TArray<OpList> OpLists;
	OpLists.Push(MoveTemp(Ops));
	Delta.SetFromOpList(MoveTemp(OpLists), View);
}

inline void AddEntityToView(EntityView& View, const Worker_EntityId EntityId)
{
	View.Add(EntityId, EntityViewElement());
}

inline void AddComponentToView(EntityView& View, const Worker_EntityId EntityId, ComponentData Data)
{
	View[EntityId].Components.Push(MoveTemp(Data));
}

inline void AddAuthorityToView(EntityView& View, const Worker_EntityId EntityId, const Worker_ComponentId ComponentId)
{
	View[EntityId].Authority.Push(ComponentId);
}

inline void PopulateViewDeltaWithComponentAdded(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                    const Worker_ComponentId ComponentId)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddComponent(EntityId, ComponentData{ ComponentId });
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline void PopulateViewDeltaWithComponentAddedWithValue(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                const Worker_ComponentId ComponentId, const double Value)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddComponent(EntityId, CreateTestComponentData(ComponentId, Value));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline void PopulateViewDeltaWithComponentUpdated(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                  const Worker_ComponentId ComponentId, const double Value)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.UpdateComponent(EntityId, CreateTestComponentUpdate(ComponentId, Value));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline void PopulateViewDeltaWithComponentRemoved(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                  const Worker_ComponentId ComponentId)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveComponent(EntityId, ComponentId);
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline void PopulateViewDeltaWithAuthorityChange(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                      const Worker_ComponentId ComponentId, const Worker_Authority Authority)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(EntityId, ComponentId, Authority);
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline void PopulateViewDeltaWithAuthorityLostTemp(ViewDelta& Delta, EntityView& View, const Worker_EntityId EntityId,
                                                      const Worker_ComponentId ComponentId)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(EntityId, ComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	OpListBuilder.SetAuthority(EntityId, ComponentId, WORKER_AUTHORITY_AUTHORITATIVE);
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
}

inline bool CompareViews(const EntityView& Lhs, const EntityView& Rhs)
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
		if (!AreEquivalent(LhsElement->Components, RhsElement->Components, CompareComponentData))
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
} // namespace SpatialGDK
