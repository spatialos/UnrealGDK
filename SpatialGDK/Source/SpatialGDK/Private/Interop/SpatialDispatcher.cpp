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
	for (auto& EntityChannelPair : NetDriver->GetEntityToActorChannelMap())
	{
		EntityChannelPair.Value->SpatialViewTick();
	}
}

bool USpatialDispatcher::IsExternalSchemaOp(Worker_Op* Op) const
{
	Worker_ComponentId ComponentId = GetComponentId(Op);
	return SpatialConstants::MIN_EXTERNAL_SCHEMA_ID <= ComponentId && ComponentId <= SpatialConstants::MAX_EXTERNAL_SCHEMA_ID;
}

void USpatialDispatcher::ProcessExternalSchemaOp(Worker_Op* Op)
{
	Worker_ComponentId ComponentId = GetComponentId(Op);
	check(ComponentId != SpatialConstants::INVALID_COMPONENT_ID);

	switch (Op->op_type)
	{
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		StaticComponentView->OnAuthorityChange(Op->authority_change);
		// Intentional fall-through
	case WORKER_OP_TYPE_ADD_COMPONENT:
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		RunCallbacks(ComponentId, Op);
		break;
	default:
		// This never happen providing the GetComponentId has the same
		// explicit cases as the switch in this method
		checkNoEntry();
		return;
	}
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

USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AddComponentOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_ADD_COMPONENT, [Callback](const Worker_Op* Op)
	{
		Callback(Op->add_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_RemoveComponentOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_REMOVE_COMPONENT, [Callback](const Worker_Op* Op)
	{
		Callback(Op->remove_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AuthorityChangeOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_AUTHORITY_CHANGE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->authority_change);
	});
}
USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_ComponentUpdateOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMPONENT_UPDATE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->component_update);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandRequestOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMMAND_REQUEST, [Callback](const Worker_Op* Op)
	{
		Callback(Op->command_request);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddOpCallback(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandResponseOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMMAND_RESPONSE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->command_response);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddGenericOpCallback(Worker_ComponentId ComponentId, Worker_OpType OpType, const TFunction<void(const Worker_Op*)>& Callback)
{
	check(SpatialConstants::MIN_EXTERNAL_SCHEMA_ID <= ComponentId && ComponentId <= SpatialConstants::MAX_EXTERNAL_SCHEMA_ID);
	const FCallbackId NewCallbackId = NextCallbackId++;
	ComponentOpTypeToCallbackMap.FindOrAdd(ComponentId).FindOrAdd(OpType).Add(UserOpCallbackData{ NewCallbackId, Callback });
	CallbackIdToDataMap.Add(NewCallbackId, CallbackIdData{ ComponentId, OpType });
	return NewCallbackId;
}

bool USpatialDispatcher::RemoveOpCallback(FCallbackId CallbackId)
{
	// Find callback ID in map and assert if it does not exist
	CallbackIdData CallbackDataToRemove = CallbackIdToDataMap.FindAndRemoveChecked(CallbackId);

	TMap<Worker_OpType, TArray<UserOpCallbackData>>* OpTypesToCallbacks = ComponentOpTypeToCallbackMap.Find(CallbackDataToRemove.ComponentId);
	if (OpTypesToCallbacks == nullptr)
	{
		return false;
	}

	TArray<UserOpCallbackData>* ComponentCallbacks = OpTypesToCallbacks->Find(CallbackDataToRemove.OpType);
	if (ComponentCallbacks == nullptr)
	{
		return false;
	}

	int32 CallbackIndex = ComponentCallbacks->IndexOfByPredicate([CallbackId](const UserOpCallbackData& Data)
	{
		return Data.Id == CallbackId;
	});
	if (CallbackIndex == INDEX_NONE)
	{
		return false;
	}

	// If removing the only callback for a component ID / op type, delete map entries as applicable
	if (OpTypesToCallbacks->Num() == 1)
	{
		if (ComponentOpTypeToCallbackMap.Num() == 1)
		{
			ComponentOpTypeToCallbackMap.Remove(CallbackDataToRemove.ComponentId);
			return true;
		}
		OpTypesToCallbacks->Remove(CallbackDataToRemove.OpType);
		return true;
	}

	ComponentCallbacks->RemoveAt(CallbackIndex);
	return true;
}

void USpatialDispatcher::RunCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op)
{
	TMap<Worker_OpType, TArray<UserOpCallbackData>>* OpTypeCallbacks = ComponentOpTypeToCallbackMap.Find(ComponentId);
	if (OpTypeCallbacks == nullptr)
	{
		return;
	}

	TArray<UserOpCallbackData>* ComponentCallbacks = OpTypeCallbacks->Find(static_cast<Worker_OpType>(Op->op_type));
	if (ComponentCallbacks == nullptr)
	{
		return;
	}

	for (UserOpCallbackData CallbackData : *ComponentCallbacks)
	{
		CallbackData.Callback(Op);
	}
}
