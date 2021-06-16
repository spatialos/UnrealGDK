// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/SpatialView/TargetView.h"

#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
void FTargetView::AddEntity(FSpatialEntityId EntityId)
{
	check(!bDisconnected);
	Builder.AddEntity(EntityId);
	View.Emplace(EntityId);
}

void FTargetView::RemoveEntity(FSpatialEntityId EntityId)
{
	check(!bDisconnected);
	// Make sure ops are generated to remove all authority and components for the removed entity.
	EntityViewElement EntityData = View.FindAndRemoveChecked(EntityId);
	for (const Worker_ComponentSetId& SetId : EntityData.Authority)
	{
		Builder.SetAuthority(EntityId, SetId, WORKER_AUTHORITY_NOT_AUTHORITATIVE, {});
	}
	for (const ComponentData& Component : EntityData.Components)
	{
		Builder.RemoveComponent(EntityId, Component.GetComponentId());
	}
	Builder.RemoveEntity(EntityId);
}

void FTargetView::AddOrSetComponent(FSpatialEntityId EntityId, ComponentData Data)
{
	check(!bDisconnected);
	EntityViewElement* EntityData = View.Find(EntityId);
	if (EntityData == nullptr)
	{
		Builder.AddEntity(EntityId);
		EntityData = &View.Emplace(EntityId);
	}
	ComponentData* Component = EntityData->Components.FindByPredicate(ComponentIdEquality{ Data.GetComponentId() });
	if (Component != nullptr)
	{
		*Component = Data.DeepCopy();
	}
	else
	{
		EntityData->Components.Add(Data.DeepCopy());
	}
	Builder.AddComponent(EntityId, MoveTemp(Data));
}

void FTargetView::UpdateComponent(FSpatialEntityId EntityId, ComponentUpdate Update)
{
	check(!bDisconnected);
	EntityViewElement& EntityData = View.FindChecked(EntityId);
	ComponentData* Component = EntityData.Components.FindByPredicate(ComponentIdEquality{ Update.GetComponentId() });
	check(Component != nullptr);
	Component->ApplyUpdate(Update);
	Builder.UpdateComponent(EntityId, MoveTemp(Update));
}

void FTargetView::RemoveComponent(FSpatialEntityId EntityId, Worker_ComponentId ComponentId)
{
	check(!bDisconnected);
	EntityViewElement& EntityData = View.FindChecked(EntityId);
	ComponentData* Component = EntityData.Components.FindByPredicate(ComponentIdEquality{ ComponentId });
	check(Component != nullptr);
	EntityData.Components.RemoveAtSwap(Component - EntityData.Components.GetData());
	Builder.RemoveComponent(EntityId, ComponentId);
}

void FTargetView::AddAuthority(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId)
{
	check(!bDisconnected);
	EntityViewElement* EntityData = View.Find(EntityId);
	if (EntityData == nullptr)
	{
		Builder.AddEntity(EntityId);
		EntityData = &View.Emplace(EntityId);
	}
	check(!EntityData->Authority.Contains(ComponentSetId));
	EntityData->Authority.Add(ComponentSetId);
	Builder.SetAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE, {});
}

void FTargetView::RemoveAuthority(FSpatialEntityId EntityId, Worker_ComponentSetId ComponentSetId)
{
	check(!bDisconnected);
	EntityViewElement& EntityData = View.FindChecked(EntityId);
	const int32 Removed = EntityData.Authority.RemoveSingleSwap(ComponentSetId);
	check(Removed);
	Builder.SetAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_NOT_AUTHORITATIVE, {});
}

void FTargetView::Disconnect(Worker_ConnectionStatusCode StatusCode, StringStorage DisconnectReason)
{
	check(!bDisconnected);
	bDisconnected = true;
	Builder.SetDisconnect(StatusCode, MoveTemp(DisconnectReason));
}

void FTargetView::AddCreateEntityCommandResponse(FSpatialEntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
												 StringStorage Message)
{
	check(!bDisconnected);
	Builder.AddCreateEntityCommandResponse(EntityId, RequestId, StatusCode, MoveTemp(Message));
}

void FTargetView::AddEntityQueryCommandResponse(Worker_RequestId RequestId, TArray<OpListEntity> Results, Worker_StatusCode StatusCode,
												StringStorage Message)
{
	check(!bDisconnected);
	Builder.AddEntityQueryCommandResponse(RequestId, MoveTemp(Results), StatusCode, MoveTemp(Message));
}

void FTargetView::AddEntityCommandRequest(FSpatialEntityId EntityId, Worker_RequestId RequestId, CommandRequest CommandRequest)
{
	check(!bDisconnected);
	Builder.AddEntityCommandRequest(EntityId, RequestId, MoveTemp(CommandRequest));
}

void FTargetView::AddEntityCommandResponse(FSpatialEntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
										   StringStorage Message)
{
	check(!bDisconnected);
	Builder.AddEntityCommandResponse(EntityId, RequestId, StatusCode, MoveTemp(Message));
}

void FTargetView::AddDeleteEntityCommandResponse(FSpatialEntityId EntityId, Worker_RequestId RequestId, Worker_StatusCode StatusCode,
												 StringStorage Message)
{
	check(!bDisconnected);
	Builder.AddDeleteEntityCommandResponse(EntityId, RequestId, StatusCode, MoveTemp(Message));
}

void FTargetView::AddReserveEntityIdsCommandResponse(FSpatialEntityId EntityId, uint32 NumberOfEntities, Worker_RequestId RequestId,
													 Worker_StatusCode StatusCode, StringStorage Message)
{
	check(!bDisconnected);
	Builder.AddReserveEntityIdsCommandResponse(EntityId, NumberOfEntities, RequestId, StatusCode, MoveTemp(Message));
}

const EntityView& FTargetView::GetView() const
{
	return View;
}

OpList FTargetView::CreateOpListFromChanges()
{
	EntityComponentOpListBuilder Temp = MoveTemp(Builder);
	Builder = EntityComponentOpListBuilder();
	return MoveTemp(Temp).CreateOpList();
}
} // namespace SpatialGDK
