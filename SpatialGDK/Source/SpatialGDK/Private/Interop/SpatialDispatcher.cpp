// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialDispatcher.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Interop/SpatialWorkerFlags.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(LogSpatialView);

void USpatialDispatcher::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Receiver = InNetDriver->Receiver;
	StaticComponentView = InNetDriver->StaticComponentView;

	// Collect all OpCallbackTemplate subclasses to register user callbacks in pipeline.
	for (TObjectIterator<UClass> OpCallbackClass; OpCallbackClass; ++OpCallbackClass)
	{
		if (OpCallbackClass->IsChildOf(UOpCallbackTemplate::StaticClass()) && *OpCallbackClass != UOpCallbackTemplate::StaticClass())
		{
			UOpCallbackTemplate* CallbackObject = NewObject<UOpCallbackTemplate>(this, *OpCallbackClass);
			CallbackObject->InternalInit(NetDriver->GetWorld(), StaticComponentView);
			UE_LOG(LogSpatialView, Log, TEXT("Registered dispatcher callback class %s to handle component ID %d."), *OpCallbackClass->GetName(), CallbackObject->GetComponentId());
			UserOpCallbacks.Add(CallbackObject->GetComponentId(), CallbackObject);
		}
	}
}

void USpatialDispatcher::ProcessOps(Worker_OpList* OpList)
{
	TArray<Worker_Op*> QueuedComponentUpdateOps;

	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];

		if (IsExternalSchemaOp(Op))
		{
			ProcessExternalSchemaOp(Op);
			continue;
		}

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
			Receiver->OnReserveEntityIdsResponse(Op->reserve_entity_ids_response);
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
			USpatialWorkerFlags::ApplyWorkerFlagUpdate(Op->flag_update);
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogSpatialView, Log, TEXT("SpatialOS Worker Log: %s"), UTF8_TO_TCHAR(Op->log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
			break;

		case WORKER_OP_TYPE_DISCONNECT:
			Receiver->OnDisconnect(Op->disconnect);
			break;

		default:
			break;
		}
	}

	for (Worker_Op* Op : QueuedComponentUpdateOps)
	{
		Receiver->OnComponentUpdate(Op->component_update);
	}

	Receiver->FlushRetryRPCs();

	// Check every channel for net ownership changes (determines ACL and component interest)
	const FActorChannelMap& ChannelMap = NetDriver->GetSpatialOSNetConnection()->ActorChannelMap();
	for (auto& Pair : ChannelMap)
	{
		if (USpatialActorChannel* Channel = Cast<USpatialActorChannel>(Pair.Value))
		{
			Channel->SpatialViewTick();
		}
	}
}

bool USpatialDispatcher::IsExternalSchemaOp(Worker_Op* Op) const
{
	Worker_ComponentId ComponentId = GetComponentId(Op);
	return  SpatialConstants::MIN_EXTERNAL_SCHEMA_ID <= ComponentId && ComponentId <= SpatialConstants::MAX_EXTERNAL_SCHEMA_ID;
}

void USpatialDispatcher::ProcessExternalSchemaOp(Worker_Op* Op)
{
	auto TryUserCallback = [&](Worker_ComponentId ComponentId, TFunction<void(UOpCallbackTemplate*)>& OpCallback)
	{
		if (UOpCallbackTemplate** UserCallbackWrapper = UserOpCallbacks.Find(ComponentId))
		{
			UOpCallbackTemplate* UserCallback = *UserCallbackWrapper;
			OpCallback(UserCallback);
		}
	};
	Worker_ComponentId ComponentId = GetComponentId(Op);
	TFunction<void(UOpCallbackTemplate*)> UserCallback;
	
	switch (Op->op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnAddComponent(Op->add_component);
		};
		break;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnRemoveComponent(Op->remove_component);
		};
		break;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnComponentUpdate(Op->component_update);
		};
		break;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		StaticComponentView->OnAuthorityChange(Op->authority_change);
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnAuthorityChange(Op->authority_change);
		};
		break;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnCommandRequest(Op->command_request);
		};
		break;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		UserCallback = [&](UOpCallbackTemplate* UserCallback)
		{
			UserCallback->OnCommandResponse(Op->command_response);
		};
		break;
	default:
		return;
	}
	TryUserCallback(ComponentId, UserCallback);
}

Worker_ComponentId USpatialDispatcher::GetComponentId(Worker_Op* Op) const
{
	switch (Op->op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		return Op->add_component.data.component_id;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		return Op->remove_component.component_id;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		return Op->component_update.update.component_id;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		return Op->authority_change.component_id;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		return Op->command_request.request.component_id;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		return Op->command_response.response.component_id;
	default:
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}
