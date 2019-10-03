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

void USpatialDispatcher::ProcessOps(const gdk::SpatialOsWorker& Worker)
{
	Receiver->OnCriticalSection(true);
	ProcessNewEntities(Worker);
	ProcessEntityComponentMessages(Worker, SpatialConstants::MIN_SPATIAL_OS_STANDARD_COMPONENT_ID);
	ProcessEntityComponentMessages(Worker, SpatialConstants::MIN_GENERATED_COMPONENT_ID);
	ProcessEntityComponentMessages(Worker, SpatialConstants::MIN_GDK_STANDARD_COMPONENT_ID);
	Receiver->OnCriticalSection(false);
	ProcessGdkCommands(Worker, SpatialConstants::MIN_GDK_STANDARD_COMPONENT_ID);
	ProcessGdkCommands(Worker, SpatialConstants::MIN_SPATIAL_OS_STANDARD_COMPONENT_ID);
	ProcessUserMessages(Worker, SpatialConstants::MIN_EXTERNAL_SCHEMA_ID);
	ProcessWorldCommandResponses(Worker);
	ProcessWorkerMessages(Worker);

	Receiver->FlushRemoveComponentOps();
	Receiver->FlushRetryRPCs();
}

void USpatialDispatcher::ProcessWorkerMessages(const gdk::SpatialOsWorker& Worker)
{
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
		UE_LOG(LogSpatialView, Log, TEXT("Disconnected"), UTF8_TO_TCHAR(Op.reason));
	}
}

void USpatialDispatcher::ProcessNewEntities(const gdk::SpatialOsWorker& Worker)
{
	for (const auto& EntityId : Worker.GetEntitiesAdded())
	{
		Receiver->OnAddEntity(Worker_AddEntityOp{EntityId});
	}
	for (const auto& EntityId : Worker.GetEntitiesRemoved())
	{
		Receiver->OnRemoveEntity(Worker_RemoveEntityOp{EntityId});
		StaticComponentView->OnRemoveEntity(EntityId);
		Receiver->RemoveComponentOpsForEntity(EntityId);
	}
}

void USpatialDispatcher::ProcessEntityComponentMessages(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId)
{
	// Components added and removed.
	for (const auto& entityComponent : Worker.GetComponentsAdded(RangeId))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_AddComponentOp Op{ entityComponent.EntityId, Data };
		StaticComponentView->OnAddComponent(Op);
		Receiver->OnAddComponent(Op);
	}
	for (const auto& entityComponent : Worker.GetComponentsRemoved(RangeId))
	{
		Receiver->OnRemoveComponent(Worker_RemoveComponentOp{ entityComponent.EntityId, entityComponent.ComponentId });
	}

	// Authority changes.
	// This should be processed after components have been added.
	for (const auto& Authority : Worker.GetAuthorityGained(RangeId))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITATIVE) };
		Receiver->OnAuthorityChange(Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLost(RangeId))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_NOT_AUTHORITATIVE) };
		Receiver->OnAuthorityChange(Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLossImminent(RangeId))
	{
		Worker_AuthorityChangeOp Op{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT) };
		Receiver->OnAuthorityChange(Op);
	}

	// Updates and events.
	for (const auto& entityComponent : Worker.GetComponentUpdates(RangeId))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_ComponentUpdateOp Op{ entityComponent.EntityId, Update };
		Receiver->OnComponentUpdate(Op);
		StaticComponentView->OnComponentUpdate(Op);
	}
	for (const auto& entityComponent : Worker.GetEvents(RangeId))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_ComponentUpdateOp Op{ entityComponent.EntityId, Update };
		Receiver->OnComponentUpdate(Op);
	}
	for (const auto& entityComponent : Worker.GetCompleteUpdates(RangeId))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_AddComponentOp Op{ entityComponent.EntityId, Data };
		StaticComponentView->OnAddComponent(Op);
		Receiver->OnAddComponent(Op);
	}
}

void USpatialDispatcher::ProcessGdkCommands(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId)
{
	for (const auto& request : Worker.GetCommandRequests(RangeId))
	{
		TArray<const char*> Attributes;
		Attributes.Reserve(request.CallerAttributeSet.size());
		for (size_t i = 0; i < request.CallerAttributeSet.size(); ++i) {
			Attributes.Emplace(request.CallerAttributeSet[i].c_str());
		}
		auto Size = static_cast<std::uint32_t>(request.CallerAttributeSet.size());
		Worker_WorkerAttributes CallerAttributes{Size, Attributes.GetData() };
		Worker_CommandRequest r{ nullptr, request.Request.GetComponentId(), request.Request.GetCommandIndex(), request.Request.GetUnderlying(), nullptr };
		Worker_CommandRequestOp Op{
			request.RequestId, request.EntityId, request.TimeoutMillis,
			request.CallerWorkerId.c_str(), CallerAttributes, r};
		Receiver->OnCommandRequest(Op);
	}
	for (const auto& response : Worker.GetCommandResponses(RangeId))
	{
		Worker_CommandResponse r{ nullptr, response.Response.GetComponentId(), response.Response.GetCommandIndex(), response.Response.GetUnderlying(), nullptr };
		Worker_CommandResponseOp Op{ 
			response.RequestId, 0, static_cast<std::uint8_t>(response.Status),
			response.Message.c_str(), r 
		};
		Receiver->OnCommandResponse(Op);
	}
}

void USpatialDispatcher::ProcessUserMessages(const gdk::SpatialOsWorker& Worker, gdk::ComponentId RangeId)
{
	// Components added and removed.
	for (const auto& entityComponent : Worker.GetComponentsAdded(RangeId))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_COMPONENT);
		Op.op.add_component = Worker_AddComponentOp{ entityComponent.EntityId, Data };
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& entityComponent : Worker.GetComponentsRemoved(RangeId))
	{
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_REMOVE_COMPONENT);
		Op.op.remove_component = Worker_RemoveComponentOp{ entityComponent.EntityId, entityComponent.ComponentId };
		ProcessExternalSchemaOp(&Op);
	}

	// Authority changes.
	// This should be processed after components have been added.
	for (const auto& Authority : Worker.GetAuthorityGained(RangeId))
	{
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_AUTHORITY_CHANGE);
		Op.op.authority_change = Worker_AuthorityChangeOp{ Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITATIVE) };
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLost(RangeId))
	{
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_AUTHORITY_CHANGE);
		Op.op.authority_change = Worker_AuthorityChangeOp { Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_NOT_AUTHORITATIVE) };
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& Authority : Worker.GetAuthorityLossImminent(RangeId))
	{
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_AUTHORITY_CHANGE);
		Op.op.authority_change = Worker_AuthorityChangeOp { Authority.EntityId, Authority.ComponentId, static_cast<std::uint8_t>(WORKER_AUTHORITY_AUTHORITY_LOSS_IMMINENT) };
		ProcessExternalSchemaOp(&Op);
	}

	// Updates and events.
	for (const auto& entityComponent : Worker.GetComponentUpdates(RangeId))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMPONENT_UPDATE);
		Op.op.component_update = Worker_ComponentUpdateOp { entityComponent.EntityId, Update };
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& entityComponent : Worker.GetEvents(RangeId))
	{
		Worker_ComponentUpdate Update{ nullptr, entityComponent.Update.GetComponentId(), entityComponent.Update.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMPONENT_UPDATE);
		Op.op.component_update = Worker_ComponentUpdateOp { entityComponent.EntityId, Update };
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& entityComponent : Worker.GetCompleteUpdates(RangeId))
	{
		Worker_ComponentData Data{ nullptr, entityComponent.Data.GetComponentId(), entityComponent.Data.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_ADD_COMPONENT);
		Op.op.add_component = Worker_AddComponentOp{ entityComponent.EntityId, Data };
		ProcessExternalSchemaOp(&Op);
	}

	// Commands.
	for (const auto& request : Worker.GetCommandRequests(RangeId))
	{
		TArray<const char*> Attributes;
		Attributes.Reserve(request.CallerAttributeSet.size());
		for (size_t i = 0; i < request.CallerAttributeSet.size(); ++i) {
			Attributes.Emplace(request.CallerAttributeSet[i].c_str());
		}
		auto Size = static_cast<std::uint32_t>(request.CallerAttributeSet.size());
		Worker_WorkerAttributes CallerAttributes{Size, Attributes.GetData() };
		Worker_CommandRequest r{ nullptr, request.Request.GetComponentId(), request.Request.GetCommandIndex(), request.Request.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_REQUEST);
		Op.op.command_request = Worker_CommandRequestOp {
			request.RequestId, request.EntityId, request.TimeoutMillis,
			request.CallerWorkerId.c_str(), CallerAttributes, r};
		ProcessExternalSchemaOp(&Op);
	}
	for (const auto& response : Worker.GetCommandResponses(RangeId))
	{
		Worker_CommandResponse r{ nullptr, response.Response.GetComponentId(), response.Response.GetCommandIndex(), response.Response.GetUnderlying(), nullptr };
		Worker_Op Op{};
		Op.op_type = static_cast<std::uint8_t>(WORKER_OP_TYPE_COMMAND_RESPONSE);
		Op.op.command_response = Worker_CommandResponseOp{ 
			response.RequestId, 0, static_cast<std::uint8_t>(response.Status),
			response.Message.c_str(), r };
		ProcessExternalSchemaOp(&Op);
	}
}

void USpatialDispatcher::ProcessWorldCommandResponses(const gdk::SpatialOsWorker& Worker)
{
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
