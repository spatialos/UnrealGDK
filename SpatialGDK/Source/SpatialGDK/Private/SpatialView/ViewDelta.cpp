// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
void ViewDelta::AddOpList(OpList Ops, TSet<EntityComponentId>& ComponentsPresent)
{
	for (uint32 i = 0; i < Ops.Count; ++i)
	{
		ProcessOp(Ops.Ops[i], ComponentsPresent);
	}
	OpLists.Add(MoveTemp(Ops));
}

bool ViewDelta::HasDisconnected() const
{
	return ConnectionStatus != 0;
}

uint8 ViewDelta::GetConnectionStatus() const
{
	check(HasDisconnected());
	return ConnectionStatus;
}

FString ViewDelta::GetDisconnectReason() const
{
	check(HasDisconnected());
	return DisconnectReason;
}

const TArray<Worker_EntityId>& ViewDelta::GetEntitiesAdded() const
{
	return EntityPresenceChanges.GetEntitiesAdded();
}

const TArray<Worker_EntityId>& ViewDelta::GetEntitiesRemoved() const
{
	return EntityPresenceChanges.GetEntitiesRemoved();
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

void ViewDelta::Clear()
{
	WorkerMessages.Empty();
	OpLists.Empty();
	AuthorityChanges.Clear();
	EntityComponentChanges.Clear();
	ConnectionStatus = 0;
}

void ViewDelta::ProcessOp(const Worker_Op& Op, TSet<EntityComponentId>& ComponentsPresent)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		ConnectionStatus = Op.op.disconnect.connection_status_code;
		DisconnectReason = FString(Op.op.disconnect.reason);
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
		EntityPresenceChanges.RemoveEntity(Op.op.remove_entity.entity_id);
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

void ViewDelta::HandleAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	AuthorityChanges.SetAuthority(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority));
}

void ViewDelta::HandleAddComponent(const Worker_AddComponentOp& Op, TSet<EntityComponentId>& ComponentsPresent)
{
	const EntityComponentId Id = { Op.entity_id, Op.data.component_id };
	if (ComponentsPresent.Contains(Id))
	{
		EntityComponentChanges.AddComponentAsUpdate(Id.EntityId, ComponentData::CreateCopy(Op.data.schema_type, Id.ComponentId));
	}
	else
	{
		ComponentsPresent.Add(Id);
		EntityComponentChanges.AddComponent(Id.EntityId, ComponentData::CreateCopy(Op.data.schema_type, Id.ComponentId));
	}
}

void ViewDelta::HandleComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	EntityComponentChanges.AddUpdate(Op.entity_id, ComponentUpdate::CreateCopy(Op.update.schema_type, Op.update.component_id));
}

void ViewDelta::HandleRemoveComponent(const Worker_RemoveComponentOp& Op, TSet<EntityComponentId>& ComponentsPresent)
{
	const EntityComponentId Id = { Op.entity_id, Op.component_id };
	// If the component has been added, remove it. Otherwise drop the op.
	if (ComponentsPresent.Remove(Id))
	{
		EntityComponentChanges.RemoveComponent(Id.EntityId, Id.ComponentId);
	}
}

} // namespace SpatialGDK
