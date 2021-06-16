// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ComponentTestUtils.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewDelta.h"
#include "SpatialView/WorkerView.h"

namespace SpatialGDK
{
inline TArray<ComponentData> CopyComponentSetOnEntity(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId,
													  const EntityView& View, const FComponentSetData& ComponentSetData)
{
	TArray<ComponentData> Components;
	const TSet<Worker_ComponentId>& ComponentSet = ComponentSetData.ComponentSets[ComponentSetId];
	for (const ComponentData& Component : View[EntityId].Components)
	{
		if (ComponentSet.Contains(Component.GetComponentId()))
		{
			Components.Emplace(Component.DeepCopy());
		}
	}
	return Components;
}

inline void SetFromOpList(ViewDelta& Delta, EntityView& View, EntityComponentOpListBuilder OpListBuilder,
						  const FComponentSetData& ComponentSetData)
{
	OpList Ops = MoveTemp(OpListBuilder).CreateOpList();
	TArray<OpList> OpLists;
	OpLists.Push(MoveTemp(Ops));
	Delta.SetFromOpList(MoveTemp(OpLists), View, ComponentSetData);
}

inline void AddEntityToView(EntityView& View, const FSpatialEntityId EntityId)
{
	View.Add(EntityId, EntityViewElement());
}

inline void AddComponentToView(EntityView& View, const FSpatialEntityId EntityId, ComponentData Data)
{
	View[EntityId].Components.Push(MoveTemp(Data));
}

inline void AddAuthorityToView(EntityView& View, const FSpatialEntityId EntityId, const Worker_ComponentId ComponentId)
{
	View[EntityId].Authority.Push(ComponentId);
}

inline void PopulateViewDeltaWithComponentAdded(ViewDelta& Delta, EntityView& View, const FSpatialEntityId EntityId, ComponentData Data)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.AddComponent(EntityId, MoveTemp(Data));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder), FComponentSetData());
}

inline void PopulateViewDeltaWithComponentUpdated(ViewDelta& Delta, EntityView& View, const FSpatialEntityId EntityId,
												  ComponentUpdate Update)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.UpdateComponent(EntityId, MoveTemp(Update));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder), FComponentSetData());
}

inline void PopulateViewDeltaWithComponentRemoved(ViewDelta& Delta, EntityView& View, const FSpatialEntityId EntityId,
												  const Worker_ComponentId ComponentId)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.RemoveComponent(EntityId, ComponentId);
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder), FComponentSetData());
}

inline void PopulateViewDeltaWithAuthorityChange(ViewDelta& Delta, EntityView& View, const FSpatialEntityId EntityId,
												 const Worker_ComponentSetId ComponentSetId, const Worker_Authority Authority,
												 const FComponentSetData& ComponentSetData)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(EntityId, ComponentSetId, Authority,
							   CopyComponentSetOnEntity(EntityId, ComponentSetId, View, ComponentSetData));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder), ComponentSetData);
}

inline void PopulateViewDeltaWithAuthorityLostTemp(ViewDelta& Delta, EntityView& View, const FSpatialEntityId EntityId,
												   const Worker_ComponentSetId ComponentSetId, const FComponentSetData& ComponentSetData)
{
	EntityComponentOpListBuilder OpListBuilder;
	OpListBuilder.SetAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_NOT_AUTHORITATIVE,
							   CopyComponentSetOnEntity(EntityId, ComponentSetId, View, ComponentSetData));
	OpListBuilder.SetAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE,
							   CopyComponentSetOnEntity(EntityId, ComponentSetId, View, ComponentSetData));
	SetFromOpList(Delta, View, MoveTemp(OpListBuilder), ComponentSetData);
}

inline bool CompareViews(const EntityView& Lhs, const EntityView& Rhs)
{
	TArray<FSpatialEntityId> LhsKeys;
	TArray<FSpatialEntityId> RhsKeys;
	Lhs.GetKeys(LhsKeys);
	Rhs.GetKeys(RhsKeys);
	if (!AreEquivalent(LhsKeys, RhsKeys, WorkerEntityIdEquality))
	{
		return false;
	}

	for (const auto& Pair : Lhs)
	{
		const FSpatialEntityId EntityId = Pair.Key;
		const EntityViewElement* LhsElement = &Pair.Value;
		const EntityViewElement* RhsElement = &Rhs[EntityId];
		if (!AreEquivalent(LhsElement->Components, RhsElement->Components, CompareComponentData))
		{
			return false;
		}

		if (!AreEquivalent(LhsElement->Authority, RhsElement->Authority, WorkerComponentIdEquality))
		{
			return false;
		}
	}

	return true;
}
} // namespace SpatialGDK
