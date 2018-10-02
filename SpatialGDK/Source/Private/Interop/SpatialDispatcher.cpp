// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialDispatcher.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"

DEFINE_LOG_CATEGORY(LogSpatialView);

void USpatialDispatcher::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	StaticComponentView = InNetDriver->StaticComponentView;
}

void USpatialDispatcher::ProcessOps(Worker_OpList* OpList)
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
			StaticComponentView->OnRemoveEntity(Op->remove_entity);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			StaticComponentView->OnAddComponent(Op->add_component);
			Receiver->OnAddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			QueuedComponentUpdateOps.Add(Op);
			StaticComponentView->OnComponentUpdate(Op->component_update);
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
			StaticComponentView->OnAuthorityChange(Op->authority_change);
			Receiver->OnAuthorityChange(Op->authority_change);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_ID_RESPONSE:
			Receiver->OnReserveEntityIdResponse(Op->reserve_entity_id_response);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			Receiver->OnCreateEntityResponse(Op->create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			Receiver->OnEntityQueryResponse(Op->entity_query_response);
			break;

		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogSpatialView, Log, TEXT("SpatialOS Worker Log: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			UE_LOG(LogSpatialView, Warning, TEXT("Disconnecting from SpatialOS: %s"), UTF8_TO_TCHAR(Op->disconnect.reason));
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

	// Check every channel for net ownership changes (determines ACL and component interest)
	for (auto& Pair : NetDriver->GetSpatialOSNetConnection()->ActorChannels)
	{
		if (USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Pair.Value))
		{
			Channel->SpatialViewTick();
		}
	}
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

// TODO UNR-640 - Need to fix for authority loss imminent
bool USpatialView::HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	if (TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority == WORKER_AUTHORITY_AUTHORITATIVE;
		}
	}

	return false;
}

improbable::UnrealMetadata* USpatialView::GetUnrealMetadata(Worker_EntityId EntityId)
{
	if (TSharedPtr<improbable::UnrealMetadata>* UnrealMetadataPtr = EntityUnrealMetadataMap.Find(EntityId))
	{
		return UnrealMetadataPtr->Get();
	}

	return nullptr;
}

improbable::EntityAcl* USpatialView::GetEntityACL(Worker_EntityId EntityId)
{
	if (TSharedPtr<improbable::EntityAcl>* EntityAclPtr = EntityACLMap.Find(EntityId))
	{
		return EntityAclPtr->Get();
	}

	return nullptr;
}

void USpatialView::OnAddComponent(const Worker_AddComponentOp& Op)
{
	if (Op.data.component_id == improbable::UnrealMetadata::ComponentId)
	{
		EntityUnrealMetadataMap.Add(Op.entity_id, MakeShared<improbable::UnrealMetadata>(Op.data));
	}

	if (Op.data.component_id == improbable::EntityAcl::ComponentId)
	{
		EntityACLMap.Add(Op.entity_id, MakeShared<improbable::EntityAcl>(Op.data));
	}
}

void USpatialView::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{
	EntityUnrealMetadataMap.Remove(Op.entity_id);
	EntityACLMap.Remove(Op.entity_id);
}

void USpatialView::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == improbable::EntityAcl::ComponentId)
	{
		EntityACLMap[Op.entity_id]->ApplyComponentUpdate(Op.update);
	}
}

void USpatialView::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{
	EntityComponentAuthorityMap.FindOrAdd(Op.entity_id).FindOrAdd(Op.component_id) = (Worker_Authority)Op.authority;
}

void USpatialView::AddEntityQueryResponse(EntityQueryFunction Func)
{
	Receiver->EntityQueryFunctions.Add(Func);
}
