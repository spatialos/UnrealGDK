// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialDispatcher.h"

#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Interop/SpatialWorkerFlags.h"
#include "UObject/UObjectIterator.h"
#include "Utils/OpUtils.h"
#include "Utils/SpatialMetrics.h"

#include "WorkerSDK/improbable/c_worker.h"

DEFINE_LOG_CATEGORY(LogSpatialView);

void USpatialDispatcher::Init(USpatialReceiver* InReceiver, USpatialStaticComponentView* InStaticComponentView, USpatialMetrics* InSpatialMetrics)
{
	Receiver = InReceiver;
	StaticComponentView = InStaticComponentView;
	SpatialMetrics = InSpatialMetrics;
}

void USpatialDispatcher::ProcessOps(Worker_OpList* OpList)
{
	for (size_t i = 0; i < OpList->op_count; ++i)
	{
		Worker_Op* Op = &OpList->ops[i];

		if (OpsToSkip.Num() != 0 &&
			OpsToSkip.Contains(Op))
		{
			OpsToSkip.Remove(Op);
			continue;
		}

		if (IsExternalSchemaOp(Op))
		{
			ProcessExternalSchemaOp(Op);
			continue;
		}

		switch (Op->op_type)
		{
		// Critical Section
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			Receiver->OnCriticalSection(Op->op.critical_section.in_critical_section != 0);
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			Receiver->OnAddEntity(Op->op.add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			Receiver->OnRemoveEntity(Op->op.remove_entity);
			StaticComponentView->OnRemoveEntity(Op->op.remove_entity.entity_id);
			Receiver->RemoveComponentOpsForEntity(Op->op.remove_entity.entity_id);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			StaticComponentView->OnAddComponent(Op->op.add_component);
			Receiver->OnAddComponent(Op->op.add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			Receiver->OnRemoveComponent(Op->op.remove_component);
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			StaticComponentView->OnComponentUpdate(Op->op.component_update);
			Receiver->OnComponentUpdate(Op->op.component_update);
			break;

		// Commands
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			Receiver->OnCommandRequest(Op->op.command_request);
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			Receiver->OnCommandResponse(Op->op.command_response);
			break;

		// Authority Change
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			Receiver->OnAuthorityChange(Op->op.authority_change);
			break;

		// World Command Responses
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			Receiver->OnReserveEntityIdsResponse(Op->op.reserve_entity_ids_response);
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			Receiver->OnCreateEntityResponse(Op->op.create_entity_response);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			Receiver->OnEntityQueryResponse(Op->op.entity_query_response);
			break;

		case WORKER_OP_TYPE_FLAG_UPDATE:
			USpatialWorkerFlags::ApplyWorkerFlagUpdate(Op->op.flag_update);
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			UE_LOG(LogSpatialView, Log, TEXT("SpatialOS Worker Log: %s"), UTF8_TO_TCHAR(Op->op.log_message.message));
			break;
		case WORKER_OP_TYPE_METRICS:
#if !UE_BUILD_SHIPPING
			SpatialMetrics->HandleWorkerMetrics(Op);
#endif
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			Receiver->OnDisconnect(Op->op.disconnect);
			break;

		default:
			break;
		}
	}

	Receiver->FlushRemoveComponentOps();
	Receiver->FlushRetryRPCs();
}

bool USpatialDispatcher::IsExternalSchemaOp(Worker_Op* Op) const
{
	Worker_ComponentId ComponentId = SpatialGDK::GetComponentId(Op);
	return SpatialConstants::MIN_EXTERNAL_SCHEMA_ID <= ComponentId && ComponentId <= SpatialConstants::MAX_EXTERNAL_SCHEMA_ID;
}

void USpatialDispatcher::ProcessExternalSchemaOp(Worker_Op* Op)
{
	Worker_ComponentId ComponentId = SpatialGDK::GetComponentId(Op);
	check(ComponentId != SpatialConstants::INVALID_COMPONENT_ID);

	switch (Op->op_type)
	{
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		StaticComponentView->OnAuthorityChange(Op->op.authority_change);
		// Intentional fall-through
	case WORKER_OP_TYPE_ADD_COMPONENT:
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
	case WORKER_OP_TYPE_COMMAND_REQUEST:
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		RunCallbacks(ComponentId, Op);
		break;
	default:
		// This should never happen providing the GetComponentId function has
		// the same explicit cases as the switch in this method
		checkNoEntry();
		return;
	}
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnAddComponent(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AddComponentOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_ADD_COMPONENT, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.add_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnRemoveComponent(Worker_ComponentId ComponentId, const TFunction<void(const Worker_RemoveComponentOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_REMOVE_COMPONENT, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.remove_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnAuthorityChange(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AuthorityChangeOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_AUTHORITY_CHANGE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.authority_change);
	});
}
USpatialDispatcher::FCallbackId USpatialDispatcher::OnComponentUpdate(Worker_ComponentId ComponentId, const TFunction<void(const Worker_ComponentUpdateOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMPONENT_UPDATE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.component_update);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnCommandRequest(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandRequestOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMMAND_REQUEST, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.command_request);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnCommandResponse(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandResponseOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMMAND_RESPONSE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->op.command_response);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::AddGenericOpCallback(Worker_ComponentId ComponentId, Worker_OpType OpType, const TFunction<void(const Worker_Op*)>& Callback)
{
	check(SpatialConstants::MIN_EXTERNAL_SCHEMA_ID <= ComponentId && ComponentId <= SpatialConstants::MAX_EXTERNAL_SCHEMA_ID);
	const FCallbackId NewCallbackId = NextCallbackId++;
	ComponentOpTypeToCallbacksMap.FindOrAdd(ComponentId).FindOrAdd(OpType).Add(UserOpCallbackData{ NewCallbackId, Callback });
	CallbackIdToDataMap.Add(NewCallbackId, CallbackIdData{ ComponentId, OpType });
	return NewCallbackId;
}

bool USpatialDispatcher::RemoveOpCallback(FCallbackId CallbackId)
{
	CallbackIdData* CallbackData = CallbackIdToDataMap.Find(CallbackId);
	if (CallbackData == nullptr)
	{
		return false;
	}

	OpTypeToCallbacksMap* OpTypesToCallbacks = ComponentOpTypeToCallbacksMap.Find(CallbackData->ComponentId);
	if (OpTypesToCallbacks == nullptr)
	{
		return false;
	}

	TArray<UserOpCallbackData>* ComponentCallbacks = OpTypesToCallbacks->Find(CallbackData->OpType);
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
	if (ComponentCallbacks->Num() == 1)
	{
		if (OpTypesToCallbacks->Num() == 1)
		{
			ComponentOpTypeToCallbacksMap.Remove(CallbackData->ComponentId);
			return true;
		}
		OpTypesToCallbacks->Remove(CallbackData->OpType);
		return true;
	}

	ComponentCallbacks->RemoveAt(CallbackIndex);
	return true;
}

void USpatialDispatcher::RunCallbacks(Worker_ComponentId ComponentId, const Worker_Op* Op)
{
	OpTypeToCallbacksMap* OpTypeCallbacks = ComponentOpTypeToCallbacksMap.Find(ComponentId);
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

void USpatialDispatcher::MarkOpToSkip(const Worker_Op* Op)
{
	OpsToSkip.Add(Op);
}

int USpatialDispatcher::GetNumOpsToSkip() const
{
	return OpsToSkip.Num();
}
