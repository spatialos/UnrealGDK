// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta.h"
#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"
#include "Containers/Set.h"

namespace SpatialGDK
{

void ViewDelta::AddOpList(TUniquePtr<AbstractOpList> OpList, TSet<EntityComponentId>& ComponentsPresent)
{
	const uint32 OpCount = OpList->GetCount();
	for (uint32 i = 0; i < OpCount; ++i)
	{
		ProcessOp((*OpList)[i], ComponentsPresent);
	}
	OpLists.Add(MoveTemp(OpList));
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityGained() const
{
	return AuthorityChanges.GetAuthorityGained();
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityLost() const
{
	return AuthorityChanges.GetAuthorityLost();
}

const TArray<EntityComponentId>& ViewDelta::GetAuthorityLostTemporarily() const
{
	return AuthorityChanges.GetAuthorityLostTemporarily();
}

const TArray<EntityComponentData>& ViewDelta::GetComponentsAdded() const
{
	return EntityComponentChanges.GetComponentsAdded();
}

const TArray<EntityComponentId>& ViewDelta::GetComponentsRemoved() const
{
	return EntityComponentChanges.GetComponentsRemoved();
}

const TArray<EntityComponentUpdate>& ViewDelta::GetUpdates() const
{
	return EntityComponentChanges.GetUpdates();
}

const TArray<EntityComponentCompleteUpdate>& ViewDelta::GetCompleteUpdates() const
{
	return EntityComponentChanges.GetCompleteUpdates();
}

const TArray<Worker_Op>& ViewDelta::GetWorkerMessages() const
{
	return WorkerMessages;
}

TUniquePtr<AbstractOpList> ViewDelta::GenerateLegacyOpList() const
{
	// Todo - refactor individual op creation to an oplist type.
	TArray<Worker_Op> OpList;

	// todo Entity added ops get created here.

	// todo Component Added ops get created here.

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLost())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		OpList.Push(Op);
	}

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_NOT_AUTHORITATIVE;
		OpList.Push(Op);
	}

	// todo Component update and remove ops get created here.

	// todo Entity removed ops get created here or below.

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityLostTemporarily())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		OpList.Push(Op);
	}

	for (const EntityComponentId& Id : AuthorityChanges.GetAuthorityGained())
	{
		Worker_Op Op = {};
		Op.op_type = WORKER_OP_TYPE_AUTHORITY_CHANGE;
		Op.op.authority_change.entity_id = Id.EntityId;
		Op.op.authority_change.component_id = Id.ComponentId;
		Op.op.authority_change.authority = WORKER_AUTHORITY_AUTHORITATIVE;
		OpList.Push(Op);
	}

	// todo Command requests ops are created here.

	// The following ops do not have ordering constraints.

	return MakeUnique<ViewDeltaLegacyOpList>(MoveTemp(OpList));
}

void ViewDelta::Clear()
{
	WorkerMessages.Empty();
	OpLists.Empty();
	AuthorityChanges.Clear();
	EntityComponentChanges.Clear();
}

void ViewDelta::ProcessOp(const Worker_Op& Op, TSet<EntityComponentId>& ComponentsPresent)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		break;
	case WORKER_OP_TYPE_FLAG_UPDATE:
	case WORKER_OP_TYPE_LOG_MESSAGE:
	case WORKER_OP_TYPE_METRICS:
		WorkerMessages.Add(Op);
		break;
	case WORKER_OP_TYPE_CRITICAL_SECTION:
		// Ignore.
		break;
	case WORKER_OP_TYPE_ADD_ENTITY:
		EntityPresenceChanges.AddEntity(Op.op.add_entity.entity_id);
		break;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		EntityPresenceChanges.AddEntity(Op.op.remove_entity.entity_id);
		break;
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
		WorkerMessages.Add(Op);
		break;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		HandleAddComponent(Op.op.add_component, ComponentsPresent);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		HandleRemoveComponent(Op.op.remove_component, ComponentsPresent);
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		HandleAuthorityChange(Op.op.authority_change);
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		HandleComponentUpdate(Op.op.component_update);
		break;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		WorkerMessages.Add(Op);
		break;
	}
}

void ViewDelta::HandleAuthorityChange(const Worker_AuthorityChangeOp& AuthorityChange)
{
	AuthorityChanges.SetAuthority(AuthorityChange.entity_id, AuthorityChange.component_id, static_cast<Worker_Authority>(AuthorityChange.authority));
}

void ViewDelta::HandleAddComponent(const Worker_AddComponentOp& Component, TSet<EntityComponentId>& ComponentsPresent)
{
	const EntityComponentId Id = { Component.entity_id, Component.data.component_id };
	if (ComponentsPresent.Contains(Id))
	{
		EntityComponentChanges.AddComponentAsUpdate(Id.EntityId, ComponentData::CreateCopy(Component.data.schema_type, Id.ComponentId));
	}
	else
	{
		ComponentsPresent.Add(Id);
		EntityComponentChanges.AddComponent(Id.EntityId, ComponentData::CreateCopy(Component.data.schema_type, Id.ComponentId));
	}
}

void ViewDelta::HandleComponentUpdate(const Worker_ComponentUpdateOp& Update)
{
	EntityComponentChanges.AddUpdate(Update.entity_id, ComponentUpdate::CreateCopy(Update.update.schema_type, Update.update.component_id));
}

void ViewDelta::HandleRemoveComponent(const Worker_RemoveComponentOp& Component, TSet<EntityComponentId>& ComponentsPresent)
{
	const EntityComponentId Id = { Component.entity_id, Component.component_id };
	// If the component has been added, remove it. Otherwise drop the op.
	if (ComponentsPresent.Remove(Id))
	{
		EntityComponentChanges.RemoveComponent(Id.EntityId, Id.ComponentId);
	}
}

}  // namespace SpatialGDK
