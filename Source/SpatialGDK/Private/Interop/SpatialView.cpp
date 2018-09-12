// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReceiver.h"

void USpatialView::Init(USpatialNetDriver* NetDriver)
{
	Receiver = NetDriver->Receiver;
}

void USpatialView::ProcessOps(Worker_OpList* OpList)
{
	TArray<Worker_Op*> QueuedComponentUpdateOps;

	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];
		switch (Op->op_type)
		{
		// Critical Section
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			Receiver->OnCriticalSection(Op->critical_section.in_critical_section != 0);
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			Receiver->OnAddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			Receiver->OnRemoveEntity(Op->remove_entity);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			OnAddComponent(Op->add_component);
			Receiver->OnAddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			Receiver->OnRemoveComponent(Op->remove_component);
			OnRemoveComponent(Op->remove_component);
			break;

		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			QueuedComponentUpdateOps.Add(Op);
			break;
		// Commands
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			Receiver->OnCommandRequest(Op->command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			Receiver->OnCommandResponse(Op->command_response);
			break;

		// Authority Change
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			OnAuthorityChange(Op->authority_change);
			Receiver->OnAuthorityChange(Op->authority_change);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			Receiver->OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			Receiver->OnCreateEntityIdResponse(Op->create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			break;


		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogTemp, Log, TEXT("SpatialOS: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogTemp, Warning, TEXT("Disconnecting from SpatialOS"), UTF8_TO_TCHAR(Op->disconnect.reason));
			break;

		default:
			break;
		}
	}

	for (Worker_Op* Op : QueuedComponentUpdateOps)
	{
		Receiver->OnComponentUpdate(Op->component_update);
	}

	Receiver->ProcessQueuedResolvedObjects();
}

Worker_Authority USpatialView::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	if (TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority;
		}
	}

	return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
}

UnrealMetadata* USpatialView::GetUnrealMetadata(Worker_EntityId EntityId)
{
	if (TSharedPtr<UnrealMetadata>* UnrealMetadataPtr = EntityUnrealMetadataMap.Find(EntityId))
	{
		return UnrealMetadataPtr->Get();
	}

	return nullptr;
}

void USpatialView::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	EntityComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}

void USpatialView::OnAddComponent(const Worker_AddComponentOp& Op)
{
	if (Op.data.component_id == UnrealMetadata::ComponentId)
	{
		EntityUnrealMetadataMap.Add(Op.entity_id, MakeShared<UnrealMetadata>(Op.data));
	}
}

void USpatialView::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (Op.component_id == UnrealMetadata::ComponentId)
	{
		EntityUnrealMetadataMap.Remove(Op.entity_id);
	}
}
