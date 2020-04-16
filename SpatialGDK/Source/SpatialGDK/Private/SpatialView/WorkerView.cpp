// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/WorkerView.h"
#include "SpatialView/MessagesToSend.h"

namespace SpatialGDK
{

WorkerView::WorkerView()
: LocalChanges(MakeUnique<MessagesToSend>())
{
}

const ViewDelta* WorkerView::GenerateViewDelta()
{
	Delta.Clear();
	for (const auto& OpList : QueuedOps)
	{
		const uint32 OpCount = OpList->GetCount();
		for (uint32 i = 0; i < OpCount; ++i)
		{
			ProcessOp((*OpList)[i]);
		}
	}

	return &Delta;
}

void WorkerView::EnqueueOpList(TUniquePtr<AbstractOpList> OpList)
{
	QueuedOps.Push(MoveTemp(OpList));
}

TUniquePtr<MessagesToSend> WorkerView::FlushLocalChanges()
{
	TUniquePtr<MessagesToSend> OutgoingMessages = MoveTemp(LocalChanges);
	LocalChanges = MakeUnique<MessagesToSend>();
	return OutgoingMessages;
}

void WorkerView::SendAddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	AddedComponents.Add(EntityComponentId{ EntityId, Data.GetComponentId() });
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Data));
}

void WorkerView::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Update));
}

void WorkerView::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	AddedComponents.Remove(EntityComponentId{ EntityId, ComponentId });
	LocalChanges->ComponentMessages.Emplace(EntityId, ComponentId);
}

void WorkerView::SendCreateEntityRequest(CreateEntityRequest Request)
{
	LocalChanges->CreateEntityRequests.Push(MoveTemp(Request));
}

void WorkerView::ProcessOp(const Worker_Op& Op)
{
	switch (static_cast<Worker_OpType>(Op.op_type))
	{
	case WORKER_OP_TYPE_DISCONNECT:
		break;
	case WORKER_OP_TYPE_FLAG_UPDATE:
		break;
	case WORKER_OP_TYPE_LOG_MESSAGE:
		break;
	case WORKER_OP_TYPE_METRICS:
		break;
	case WORKER_OP_TYPE_CRITICAL_SECTION:
		break;
	case WORKER_OP_TYPE_ADD_ENTITY:
		break;
	case WORKER_OP_TYPE_REMOVE_ENTITY:
		break;
	case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
		break;
	case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
		HandleCreateEntityResponse(Op.op.create_entity_response);
		break;
	case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
		break;
	case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
		break;
	case WORKER_OP_TYPE_ADD_COMPONENT:
		HandleAddComponent(Op.op.add_component);
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		HandleRemoveComponent(Op.op.remove_component);
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		HandleAuthorityChange(Op.op.authority_change);
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		HandleComponentUpdate(Op.op.component_update);
		break;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		break;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		break;
	}
}

void WorkerView::HandleAuthorityChange(const Worker_AuthorityChangeOp& AuthorityChange)
{
	Delta.SetAuthority(AuthorityChange.entity_id, AuthorityChange.component_id, static_cast<Worker_Authority>(AuthorityChange.authority));
}

void WorkerView::HandleCreateEntityResponse(const Worker_CreateEntityResponseOp& Response)
{
	Delta.AddCreateEntityResponse(CreateEntityResponse{
		Response.request_id,
		static_cast<Worker_StatusCode>(Response.status_code),
		FString{Response.message},
		Response.entity_id
	});
}

void WorkerView::HandleAddComponent(const Worker_AddComponentOp& Component)
{
	const EntityComponentId Id = { Component.entity_id, Component.data.component_id };
	if (AddedComponents.Contains(Id))
	{
		Delta.AddComponentAsUpdate(Id.EntityId, ComponentData::CreateCopy(Component.data.schema_type, Id.ComponentId));
	}
	else
	{
		AddedComponents.Add(Id);
		Delta.AddComponent(Id.EntityId, ComponentData::CreateCopy(Component.data.schema_type, Id.ComponentId));
	}
}

void WorkerView::HandleComponentUpdate(const Worker_ComponentUpdateOp& Update)
{
	Delta.AddUpdate(Update.entity_id, ComponentUpdate::CreateCopy(Update.update.schema_type, Update.update.component_id));
}

void WorkerView::HandleRemoveComponent(const Worker_RemoveComponentOp& Component)
{
	const EntityComponentId Id = { Component.entity_id, Component.component_id };
	// If the component has been added, remove it. Otherwise drop the op.
	if (AddedComponents.Remove(Id))
	{
		Delta.RemoveComponent(Id.EntityId, Id.ComponentId);
	}
}
}  // namespace SpatialGDK
