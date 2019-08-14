// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialDispatcher.h"

#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Interop/SpatialWorkerFlags.h"
#include "Utils/OpUtils.h"
#include "Utils/SpatialMetrics.h"

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
			Receiver->OnCriticalSection(Op->critical_section.in_critical_section != 0);
			break;

		// Entity Lifetime
		case WORKER_OP_TYPE_ADD_ENTITY:
			Receiver->OnAddEntity(Op->add_entity);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			Receiver->OnRemoveEntity(Op->remove_entity);
			StaticComponentView->OnRemoveEntity(Op->remove_entity.entity_id);
			Receiver->RemoveComponentOpsForEntity(Op->remove_entity.entity_id);
			break;

		// Components
		case WORKER_OP_TYPE_ADD_COMPONENT:
			StaticComponentView->OnAddComponent(Op->add_component);
			Receiver->OnAddComponent(Op->add_component);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			Receiver->OnRemoveComponent(Op->remove_component);
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			StaticComponentView->OnComponentUpdate(Op->component_update);
			Receiver->OnComponentUpdate(Op->component_update);
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
#if !UE_BUILD_SHIPPING
			SpatialMetrics->HandleWorkerMetrics(Op);
#endif
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			Receiver->OnDisconnect(Op->disconnect);
			break;

		default:
			break;
		}
	}

	Receiver->FlushRemoveComponentOps();
	Receiver->FlushRetryRPCs();
}

void USpatialDispatcher::ProcessOps(const gdk::SpatialOsWorker& Worker)
{
	Receiver->OnCriticalSection(true);
	for (const auto& Entity : Worker.GetEntitiesAdded())
	{
		Receiver->OnAddEntity(Worker_AddEntityOp{Entity.GetEntityId()});
	}
	for (const auto& entityComponent : Worker.GetComponentsAdded(1))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_AddComponentOp Op{ entityComponent.EntityId, Data };
		StaticComponentView->OnAddComponent(Op);
		Receiver->OnAddComponent(Op);
	}
	for (const auto& entityComponent : Worker.GetComponentsRemoved(1))
	{
		Receiver->OnRemoveComponent(Worker_RemoveComponentOp{ entityComponent.EntityId, entityComponent.ComponentId });
	}
	for (const auto& EntityId : Worker.GetEntitiesRemoved())
	{
		Receiver->OnRemoveEntity(Worker_RemoveEntityOp{EntityId});
		StaticComponentView->OnRemoveEntity(EntityId);
		Receiver->RemoveComponentOpsForEntity(EntityId);
	}
	Receiver->OnCriticalSection(false);

	for (const auto& Authority : Worker.GetAuthorityGained(1))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITATIVE) };
		Receiver->OnAuthorityChange(Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLost(1))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_NOT_AUTHORITATIVE) };
		Receiver->OnAuthorityChange(Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLossImminent(1))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT) };
		Receiver->OnAuthorityChange(Op);
	}

	for (const auto& response : Worker.GetReserveEntityIdsResponses())
	{
		Worker_ReserveEntityIdsResponseOp Op{ response.RequestId, static_cast<std::uint8_t>(response.StatusCode), 
			response.Message.c_str(), response.FirstEntityId, response.NumberOfIds };
		Receiver->OnReserveEntityIdsResponse(Op);
	}
	for (const auto& response : Worker.GetCreateEntityResponses())
	{
		Worker_CreateEntityResponseOp Op{ response.RequestId, static_cast<std::uint8_t>(response.StatusCode), 
			response.Message.c_str(), response.EntityId };
		Receiver->OnCreateEntityResponse(Op);
	}
	for (const auto& response : Worker.GetEntityQueryResponses())
	{
		std::vector<Worker_Entity> Entities(response.Entities.size());
		std::vector<std::vector<Worker_ComponentData>> Data(response.Entities.size());
		for (size_t i = 0; i < response.Entities.size(); ++i)
		{
			const auto& components = response.Entities[i].GetEntityState().GetComponents();
			Entities[i].component_count = components.size();
			Entities[i].entity_id = response.Entities[i].GetEntityId();
			Data[i] = std::vector<Worker_ComponentData>( components.size() );
			std::transform(components.begin(), components.end(), Data[i].begin(), [](const gdk::ComponentData& data) -> Worker_ComponentData
			{
				return Worker_ComponentData{ nullptr, data.GetComponentId(), data.GetUnderlying(), nullptr };
			});
			Entities[i].components = Data[i].data();

		}
		Worker_EntityQueryResponseOp Op{ response.RequestId, static_cast<std::uint8_t>(response.StatusCode),
			response.Message.c_str(), response.ResultCount, Entities.data() };
		Receiver->OnEntityQueryResponse(Op);
	}
	for (const auto& request : Worker.GetCommandRequests(1))
	{
		const char** Attributes = new const char*[request.CallerAttributeSet.size()];
		for (size_t i = 0; i < request.CallerAttributeSet.size(); ++i) {
			Attributes[i] = request.CallerAttributeSet[i].c_str();
		}
		auto Size = static_cast<std::uint32_t>(request.CallerAttributeSet.size());
		Worker_WorkerAttributes CallerAttributes{Size, Attributes };
		Worker_CommandRequest r{ nullptr, request.Request.GetComponentId(), request.Request.GetUnderlying(), nullptr };
		Worker_CommandRequestOp Op{
			request.RequestId, request.EntityId, request.TimeoutMillis,
			request.CallerWorkerId.c_str(), CallerAttributes, r};
		Receiver->OnCommandRequest(Op);
		delete[] Attributes;
	}
	for (const auto& response : Worker.GetCommandResponses(1))
	{
		Worker_CommandResponse r{ nullptr, response.Response.GetComponentId(), response.Response.GetUnderlying(), nullptr };
		Worker_CommandResponseOp Op{ 
			response.RequestId, 0, static_cast<std::uint8_t>(response.Status),
			response.Message.c_str(), r, response.CommandId 
		};
		Receiver->OnCommandResponse(Op);
	}
	for (const auto& flags : Worker.GetFlagChanges())
	{
		USpatialWorkerFlags::ApplyWorkerFlagUpdate(flags);
	}
	for (const auto& logs : Worker.GetLogsReceived())
	{
		UE_LOG(LogSpatialView, Log, TEXT("SpatialOS Worker Log: %s"), UTF8_TO_TCHAR(logs.message));
	}
	if (Worker.HasConnectionStatusChanged()) {
		Worker_DisconnectOp Op{ static_cast<std::uint8_t>(Worker.GetConnectionStatusCode()), Worker.GetConnectionMessage().c_str() };
			Receiver->OnDisconnect(Op);
	}

		//if (IsExternalSchemaOp(Op))
		//{
		//	ProcessExternalSchemaOp(Op);
		//	continue;
		//}

	for (const auto& entityComponent : Worker.GetComponentUpdates(1))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_ComponentUpdateOp Op{ entityComponent.EntityId, Update };
		Receiver->OnComponentUpdate(Op);
		StaticComponentView->OnComponentUpdate(Op);
	}
	for (const auto& entityComponent : Worker.GetEvents(1))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_ComponentUpdateOp Op{ entityComponent.EntityId, Update };
		Receiver->OnComponentUpdate(Op);
	}
	for (const auto& entityComponent : Worker.GetCompleteUpdates(1))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_AddComponentOp Op{ entityComponent.EntityId, Data };
		StaticComponentView->OnAddComponent(Op);
		Receiver->OnAddComponent(Op);
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
		Callback(Op->add_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnRemoveComponent(Worker_ComponentId ComponentId, const TFunction<void(const Worker_RemoveComponentOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_REMOVE_COMPONENT, [Callback](const Worker_Op* Op)
	{
		Callback(Op->remove_component);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnAuthorityChange(Worker_ComponentId ComponentId, const TFunction<void(const Worker_AuthorityChangeOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_AUTHORITY_CHANGE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->authority_change);
	});
}
USpatialDispatcher::FCallbackId USpatialDispatcher::OnComponentUpdate(Worker_ComponentId ComponentId, const TFunction<void(const Worker_ComponentUpdateOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMPONENT_UPDATE, [Callback](const Worker_Op* Op)
	{
		Callback(Op->component_update);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnCommandRequest(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandRequestOp&)>& Callback)
{
	return AddGenericOpCallback(ComponentId, WORKER_OP_TYPE_COMMAND_REQUEST, [Callback](const Worker_Op* Op)
	{
		Callback(Op->command_request);
	});
}

USpatialDispatcher::FCallbackId USpatialDispatcher::OnCommandResponse(Worker_ComponentId ComponentId, const TFunction<void(const Worker_CommandResponseOp&)>& Callback)
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
