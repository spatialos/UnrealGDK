// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialFastArrayNetSerialize.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "Schema/DynamicComponent.h"
#include "Schema/MigrationDiagnostic.h"
#include "Schema/RPCPayload.h"
#include "Schema/Restricted.h"
#include "Schema/SpawnData.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialConstants.h"
#include "Utils/ComponentReader.h"
#include "Utils/ErrorCodeRemapping.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialMetrics.h"

DEFINE_LOG_CATEGORY(LogSpatialReceiver);

DECLARE_CYCLE_STAT(TEXT("PendingOpsOnChannel"), STAT_SpatialPendingOpsOnChannel, STATGROUP_SpatialNet);

DECLARE_CYCLE_STAT(TEXT("Receiver LeaveCritSection"), STAT_ReceiverLeaveCritSection, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveEntity"), STAT_ReceiverRemoveEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AddComponent"), STAT_ReceiverAddComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ComponentUpdate"), STAT_ReceiverComponentUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyData"), STAT_ReceiverApplyData, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyHandover"), STAT_ReceiverApplyHandover, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver HandleRPC"), STAT_ReceiverHandleRPC, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandRequest"), STAT_ReceiverCommandRequest, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CommandResponse"), STAT_ReceiverCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver AuthorityChange"), STAT_ReceiverAuthChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReserveEntityIds"), STAT_ReceiverReserveEntityIds, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver CreateEntityResponse"), STAT_ReceiverCreateEntityResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver EntityQueryResponse"), STAT_ReceiverEntityQueryResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver SystemEntityCommandResponse"), STAT_ReceiverSystemEntityCommandResponse, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver FlushRemoveComponents"), STAT_ReceiverFlushRemoveComponents, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ReceiveActor"), STAT_ReceiverReceiveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver RemoveActor"), STAT_ReceiverRemoveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Receiver ApplyRPC"), STAT_ReceiverApplyRPC, STATGROUP_SpatialNet);
using namespace SpatialGDK;

namespace // Anonymous namespace
{
struct RepNotifyCall
{
	USpatialActorChannel* Channel;
	UObject* Object;
	TArray<GDK_PROPERTY(Property)*> Notifies;
};
#if DO_CHECK
FUsageLock ObjectRefToRepStateUsageLock; // A debug helper to trigger an ensure if something weird happens (re-entrancy)
#endif
} // namespace

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialRPCService* InRPCService,
							SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	GlobalStateManager = InNetDriver->GlobalStateManager;
	TimerManager = InTimerManager;
	RPCService = InRPCService;
	EventTracer = InEventTracer;
}

void USpatialReceiver::FlushRemoveComponentOps()
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverFlushRemoveComponents);

	for (const auto& Op : QueuedRemoveComponentOps)
	{
		ProcessRemoveComponent(Op);
	}

	QueuedRemoveComponentOps.Empty();
}

USpatialActorChannel* USpatialReceiver::GetOrRecreateChannelForDomantActor(AActor* Actor, Worker_EntityId EntityID)
{
	// Receive would normally create channel in ReceiveActor - this function is used to recreate the channel after waking up a dormant actor
	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(Actor);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	check(!Channel->bCreatingNewEntity);
	check(Channel->GetEntityId() == EntityID);

	NetDriver->RemovePendingDormantChannel(Channel);
	NetDriver->UnregisterDormantEntityId(EntityID);

	return Channel;
}

void USpatialReceiver::ProcessRemoveComponent(const Worker_RemoveComponentOp& Op)
{
	if (IsEntityWaitingForAsyncLoad(Op.entity_id))
	{
		QueueRemoveComponentOpForAsyncLoad(Op);
		return;
	}

	// We want to do nothing for RemoveComponent ops for which we never received a corresponding
	// AddComponent op. This can happen because of the worker SDK generating a RemoveComponent op
	// when a worker receives authority over a component without having already received the
	// AddComponent op. The generation is a known part of the worker SDK we need to tolerate for
	// enabling dynamic components, and having authority ACL entries without having the component
	// data present on an entity is permitted as part of our Unreal dynamic component implementation.
	if (!StaticComponentView->HasComponent(Op.entity_id, Op.component_id))
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(Op.entity_id).Get()))
	{
		FUnrealObjectRef ObjectRef(Op.entity_id, Op.component_id);
		if (Op.component_id == SpatialConstants::DORMANT_COMPONENT_ID)
		{
			GetOrRecreateChannelForDomantActor(Actor, Op.entity_id);
		}
		else if (UObject* Object = PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(Op.entity_id))
			{
				TWeakObjectPtr<UObject> WeakPtr(Object);
				Channel->OnSubobjectDeleted(ObjectRef, Object, WeakPtr);

				Actor->OnSubobjectDestroyFromReplication(Object);

				Object->PreDestroyFromReplication();
				Object->MarkPendingKill();

				PackageMap->RemoveSubobject(FUnrealObjectRef(Op.entity_id, Op.component_id));
			}
		}
	}
}

void USpatialReceiver::UpdateShadowData(Worker_EntityId EntityId)
{
	USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId);
	ActorChannel->UpdateShadowData();
}

void USpatialReceiver::HandlePlayerLifecycleAuthority(const Worker_ComponentSetAuthorityChangeOp& Op, APlayerController* PlayerController)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("HandlePlayerLifecycleAuthority for PlayerController %s."),
		   *AActor::GetDebugName(PlayerController));

	// Server initializes heartbeat logic based on its authority over the position component,
	// client does the same for heartbeat component
	if ((NetDriver->IsServer() && Op.component_set_id == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
		|| (!NetDriver->IsServer() && Op.component_set_id == SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID))
	{
		if (Op.authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				if (NetDriver->IsServer())
				{
					AuthorityPlayerControllerConnectionMap.Add(Op.entity_id, Connection);
				}
				Connection->InitHeartbeat(TimerManager, Op.entity_id);
			}
		}
		else if (Op.authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
		{
			if (NetDriver->IsServer())
			{
				AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
			}
			if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
			{
				Connection->DisableHeartbeat();
			}
		}
	}
}

void USpatialReceiver::OnCommandRequest(const Worker_Op& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandRequest);

	const Worker_CommandRequestOp& CommandRequestOp = Op.op.command_request;
	const Worker_CommandRequest& Request = CommandRequestOp.request;
	const Worker_EntityId EntityId = CommandRequestOp.entity_id;
	const Worker_ComponentId ComponentId = Request.component_id;
	const Worker_RequestId RequestId = CommandRequestOp.request_id;
	const Schema_FieldId CommandIndex = Request.command_index;

	if (IsEntityWaitingForAsyncLoad(CommandRequestOp.entity_id))
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("USpatialReceiver::OnCommandRequest: Actor class async loading, cannot handle command. Entity %lld, Class %s"),
			   EntityId, *EntitiesWaitingForAsyncLoad[EntityId].ClassPath);
		Sender->SendCommandFailure(RequestId, TEXT("Target actor async loading."), FSpatialGDKSpanId(Op.span_id));
		return;
	}

	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID
		&& CommandIndex == SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnRequestOnServer(CommandRequestOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SPAWN_PLAYER_COMMAND"), RequestId),
									Op.span_id, 1);
		}

		return;
	}
	else if (ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID
			 && CommandIndex == SpatialConstants::SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND_ID)
	{
		NetDriver->PlayerSpawner->ReceiveForwardedPlayerSpawnRequest(CommandRequestOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND"), RequestId),
				Op.span_id, 1);
		}

		return;
	}
	else if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID
			 && CommandIndex == SpatialConstants::MIGRATION_DIAGNOSTIC_COMMAND_ID)
	{
		check(NetDriver != nullptr);
		check(NetDriver->Connection != nullptr);

		AActor* BlockingActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));
		if (IsValid(BlockingActor))
		{
			Worker_CommandResponse Response = MigrationDiagnostic::CreateMigrationDiagnosticResponse(NetDriver, EntityId, BlockingActor);

			Sender->SendCommandResponse(RequestId, Response, FSpatialGDKSpanId(Op.span_id));
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Warning,
				   TEXT("Migration diaganostic log failed because cannot retreive actor for entity (%llu) on authoritative worker %s"),
				   EntityId, *NetDriver->Connection->GetWorkerId());
		}

		return;
	}
#if WITH_EDITOR
	else if (ComponentId == SpatialConstants::GSM_SHUTDOWN_COMPONENT_ID
			 && CommandIndex == SpatialConstants::SHUTDOWN_MULTI_PROCESS_REQUEST_ID)
	{
		NetDriver->GlobalStateManager->ReceiveShutdownMultiProcessRequest();

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SHUTDOWN_MULTI_PROCESS_REQUEST"), RequestId), Op.span_id, 1);
		}

		return;
	}
#endif // WITH_EDITOR
#if !UE_BUILD_SHIPPING
	else if (ComponentId == SpatialConstants::DEBUG_METRICS_COMPONENT_ID)
	{
		switch (CommandIndex)
		{
		case SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStartRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID:
			NetDriver->SpatialMetrics->OnStopRPCMetricsCommand();
			break;
		case SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID:
		{
			Schema_Object* Payload = Schema_GetCommandRequestObject(CommandRequestOp.request.schema_type);
			NetDriver->SpatialMetrics->OnModifySettingCommand(Payload);
			break;
		}
		default:
			UE_LOG(LogSpatialReceiver, Error, TEXT("Unknown command index for DebugMetrics component: %d, entity: %lld"), CommandIndex,
				   EntityId);
			break;
		}

		Sender->SendEmptyCommandResponse(ComponentId, CommandIndex, RequestId, FSpatialGDKSpanId(Op.span_id));
		return;
	}
#endif // !UE_BUILD_SHIPPING
}

void USpatialReceiver::OnCommandResponse(const Worker_Op& Op)
{
	const Worker_CommandResponseOp& CommandResponseOp = Op.op.command_response;
	const Worker_CommandResponse& Repsonse = CommandResponseOp.response;
	const Worker_ComponentId ComponentId = Repsonse.component_id;
	const Worker_RequestId RequestId = CommandResponseOp.request_id;

	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandResponse);
	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnResponseOnClient(CommandResponseOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SPAWN_PLAYER_COMMAND"), RequestId),
									Op.span_id, 1);
		}

		return;
	}
	else if (ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceiveForwardPlayerSpawnResponse(CommandResponseOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND"), RequestId),
				Op.span_id, 1);
		}

		return;
	}
	else if (Op.op.command_response.response.component_id == SpatialConstants::WORKER_COMPONENT_ID)
	{
		OnSystemEntityCommandResponse(Op.op.command_response);
		return;
	}
	else if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID)
	{
		check(NetDriver != nullptr);
		check(NetDriver->Connection != nullptr);

		if (CommandResponseOp.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Migration diaganostic log failed, status code %i."), CommandResponseOp.status_code);
			return;
		}

		Schema_Object* ResponseObject = Schema_GetCommandResponseObject(CommandResponseOp.response.schema_type);
		Worker_EntityId EntityId = Schema_GetInt64(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
		AActor* BlockingActor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));
		if (IsValid(BlockingActor))
		{
			FString MigrationDiagnosticLog = MigrationDiagnostic::CreateMigrationDiagnosticLog(NetDriver, ResponseObject, BlockingActor);
			if (!MigrationDiagnosticLog.IsEmpty())
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("%s"), *MigrationDiagnosticLog);
			}
		}
		else
		{
			UE_LOG(LogSpatialReceiver, Warning, TEXT("Migration diaganostic log failed because blocking actor (%llu) is not valid."),
				   EntityId);
		}

		return;
	}
}

void USpatialReceiver::ReceiveWorkerDisconnectResponse(const Worker_CommandResponseOp& Op)
{
	if (SystemEntityCommandDelegate* RequestDelegate = SystemEntityCommandDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Executing ReceiveWorkerDisconnectResponse with delegate, request id: %d, message: %s"),
			   Op.request_id, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received ReceiveWorkerDisconnectResponse but with no delegate set, request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::ReceiveClaimPartitionResponse(const Worker_CommandResponseOp& Op)
{
	const Worker_PartitionId PartitionId = PendingPartitionAssignments.FindAndRemoveChecked(Op.request_id);

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Error,
			   TEXT("ClaimPartition command failed for a reason other than timeout. "
					"This is fatal. Partition entity: %lld. Reason: %s"),
			   PartitionId, UTF8_TO_TCHAR(Op.message));
		return;
	}

	UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
		   TEXT("ClaimPartition command succeeded. "
				"Worker sytem entity: %lld. Partition entity: %lld"),
		   Op.entity_id, PartitionId);
}

void USpatialReceiver::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverReserveEntityIds);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("ReserveEntityIds request failed: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("ReserveEntityIds request succeeded: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}

	if (ReserveEntityIDsDelegate* RequestDelegate = ReserveEntityIDsDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Log,
			   TEXT("Executing ReserveEntityIdsResponse with delegate, request id: %d, first entity id: %lld, message: %s"), Op.request_id,
			   Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
		ReserveEntityIDsDelegates.Remove(Op.request_id);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received ReserveEntityIdsResponse but with no delegate set, request id: %d, first entity id: %lld, message: %s"),
			   Op.request_id, Op.first_entity_id, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::OnCreateEntityResponse(const Worker_Op& Op)
{
	const Worker_CreateEntityResponseOp& CreateEntityResponseOp = Op.op.create_entity_response;
	const Worker_EntityId EntityId = CreateEntityResponseOp.entity_id;
	const Worker_RequestId RequestId = CreateEntityResponseOp.request_id;
	const uint8_t StatusCode = CreateEntityResponseOp.status_code;

	SCOPE_CYCLE_COUNTER(STAT_ReceiverCreateEntityResponse);
	switch (static_cast<Worker_StatusCode>(StatusCode))
	{
	case WORKER_STATUS_CODE_SUCCESS:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request succeeded. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	case WORKER_STATUS_CODE_TIMEOUT:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request timed out. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	case WORKER_STATUS_CODE_APPLICATION_ERROR:
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Create entity request failed. "
					"Either the reservation expired, the entity already existed, or the entity was invalid. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	default:
		UE_LOG(LogSpatialReceiver, Error,
			   TEXT("Create entity request failed. This likely indicates a bug in the Unreal GDK and should be reported. "
					"Request id: %d, entity id: %lld, message: %s"),
			   RequestId, EntityId, UTF8_TO_TCHAR(CreateEntityResponseOp.message));
		break;
	}

	if (CreateEntityDelegate* Delegate = CreateEntityDelegates.Find(RequestId))
	{
		Delegate->ExecuteIfBound(CreateEntityResponseOp);
		CreateEntityDelegates.Remove(RequestId);
	}

	TWeakObjectPtr<USpatialActorChannel> Channel = PopPendingActorRequest(RequestId);

	// It's possible for the ActorChannel to have been closed by the time we receive a response. Actor validity is checked within the
	// channel.
	if (Channel.IsValid())
	{
		Channel->OnCreateEntityResponse(CreateEntityResponseOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCreateEntitySuccess(Channel->Actor, EntityId), Op.span_id, 1);
		}
	}
	else if (Channel.IsStale())
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Received CreateEntityResponse for actor which no longer has an actor channel: "
					"request id: %d, entity id: %lld. This should only happen in the case where we attempt to delete the entity before we "
					"have authority. "
					"The entity will therefore be deleted once authority is gained."),
			   RequestId, EntityId);

		FString Message =
			FString::Printf(TEXT("Stale Actor Channel - tried to delete entity before gaining authority. Actor - %s EntityId - %d"),
							*Channel->Actor->GetName(), EntityId);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(Message), Op.span_id, 1);
		}
	}
	else if (EventTracer != nullptr)
	{
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(TEXT("Create entity response unknown error")), Op.span_id,
								1);
	}
}

void USpatialReceiver::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverEntityQueryResponse);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("EntityQuery failed: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}

	if (EntityQueryDelegate* RequestDelegate = EntityQueryDelegates.Find(Op.request_id))
	{
		UE_LOG(LogSpatialReceiver, Verbose,
			   TEXT("Executing EntityQueryResponse with delegate, request id: %d, number of entities: %d, message: %s"), Op.request_id,
			   Op.result_count, UTF8_TO_TCHAR(Op.message));
		RequestDelegate->ExecuteIfBound(Op);
	}
	else
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received EntityQueryResponse but with no delegate set, request id: %d, number of entities: %d, message: %s"),
			   Op.request_id, Op.result_count, UTF8_TO_TCHAR(Op.message));
	}
}

void USpatialReceiver::OnSystemEntityCommandResponse(const Worker_CommandResponseOp& Op)
{
	SCOPE_CYCLE_COUNTER(STAT_ReceiverSystemEntityCommandResponse);
	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialReceiver, Error, TEXT("SystemEntityCommand failed: request id: %d, message: %s"), Op.request_id,
			   UTF8_TO_TCHAR(Op.message));
	}

	switch (Op.response.command_index)
	{
	case SpatialConstants::WORKER_DISCONNECT_COMMAND_ID:
	{
		ReceiveWorkerDisconnectResponse(Op);
		return;
	}
	case SpatialConstants::WORKER_CLAIM_PARTITION_COMMAND_ID:
	{
		ReceiveClaimPartitionResponse(Op);
		return;
	}
	default:
		checkNoEntry();
	}
}

void USpatialReceiver::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Add(RequestId, Channel);
}

void USpatialReceiver::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<FReliableRPCForRetry> ReliableRPC)
{
	PendingReliableRPCs.Add(RequestId, ReliableRPC);
}

void USpatialReceiver::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, MoveTemp(Delegate));
}

void USpatialReceiver::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate)
{
	ReserveEntityIDsDelegates.Add(RequestId, MoveTemp(Delegate));
}

void USpatialReceiver::AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate)
{
	CreateEntityDelegates.Add(RequestId, MoveTemp(Delegate));
}

void USpatialReceiver::AddSystemEntityCommandDelegate(Worker_RequestId RequestId, SystemEntityCommandDelegate Delegate)
{
	SystemEntityCommandDelegates.Add(RequestId, MoveTemp(Delegate));
}

TWeakObjectPtr<USpatialActorChannel> USpatialReceiver::PopPendingActorRequest(Worker_RequestId RequestId)
{
	TWeakObjectPtr<USpatialActorChannel>* ChannelPtr = PendingActorRequests.Find(RequestId);
	if (ChannelPtr == nullptr)
	{
		return nullptr;
	}
	TWeakObjectPtr<USpatialActorChannel> Channel = *ChannelPtr;
	PendingActorRequests.Remove(RequestId);
	return Channel;
}

void USpatialReceiver::OnDisconnect(uint8 StatusCode, const FString& Reason)
{
	if (GEngine != nullptr)
	{
		GEngine->BroadcastNetworkFailure(NetDriver->GetWorld(), NetDriver, ENetworkFailure::FromDisconnectOpStatusCode(StatusCode), Reason);
	}
}

bool USpatialReceiver::IsPendingOpsOnChannel(USpatialActorChannel& Channel)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialPendingOpsOnChannel);

	// Don't allow Actors to go dormant if they have any pending operations waiting on their channel

	for (const auto& RefMap : Channel.ObjectReferenceMap)
	{
		if (RefMap.Value.HasUnresolved())
		{
			return true;
		}
	}

	for (const auto& ActorRequest : PendingActorRequests)
	{
		if (ActorRequest.Value == &Channel)
		{
			return true;
		}
	}

	return false;
}

void USpatialReceiver::ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(),
		   *ObjectRef.ToString());

	ResolveIncomingOperations(Object, ObjectRef);

	// When resolving an Actor that should uniquely exist in a deployment, e.g. GameMode, GameState, LevelScriptActors, we also
	// resolve using class path (in case any properties were set from a server that hasn't resolved the Actor yet).
	if (FUnrealObjectRef::ShouldLoadObjectFromClassPath(Object))
	{
		FUnrealObjectRef ClassObjectRef = FUnrealObjectRef::GetRefFromObjectClassPath(Object, PackageMap);
		if (ClassObjectRef.IsValid())
		{
			ResolveIncomingOperations(Object, ClassObjectRef);
		}
	}

	// TODO: UNR-1650 We're trying to resolve all queues, which introduces more overhead.
	RPCService->ProcessIncomingRPCs();
}

void USpatialReceiver::ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops - UNR:582

	TSet<FChannelObjectPair>* TargetObjectSet = ObjectRefToRepStateMap.Find(ObjectRef);
	if (!TargetObjectSet)
	{
		return;
	}

	UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolving incoming operations depending on object ref %s, resolved object: %s"),
		   *ObjectRef.ToString(), *Object->GetName());

	/* Rep-notify can modify ObjectRefToRepStateMap in some situations which can cause the TSet to access
	invalid memory if a) the set is removed or b) the TMap containing the TSet is reallocated. So to fix this
	we just defer the rep-notify calls to the end of this function. */
	TArray<RepNotifyCall> RepNotifyCalls;

	for (auto ChannelObjectIter = TargetObjectSet->CreateIterator(); ChannelObjectIter; ++ChannelObjectIter)
	{
		GDK_ENSURE_NO_MODIFICATIONS(ObjectRefToRepStateUsageLock);

		USpatialActorChannel* DependentChannel = ChannelObjectIter->Key.Get();
		if (!DependentChannel)
		{
			ChannelObjectIter.RemoveCurrent();
			continue;
		}

		UObject* ReplicatingObject = ChannelObjectIter->Value.Get();

		if (!ReplicatingObject)
		{
			if (DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value))
			{
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				ChannelObjectIter.RemoveCurrent();
			}
			continue;
		}

		FSpatialObjectRepState* RepState = DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value);
		if (!RepState || !RepState->UnresolvedRefs.Contains(ObjectRef))
		{
			continue;
		}

		// Check whether the resolved object has been torn off, or is on an actor that has been torn off.
		if (AActor* AsActor = Cast<AActor>(ReplicatingObject))
		{
			if (AsActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Actor to be resolved was torn off, so ignoring incoming operations. Object ref: %s, resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}
		else if (AActor* OuterActor = ReplicatingObject->GetTypedOuter<AActor>())
		{
			if (OuterActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Owning Actor of the object to be resolved was torn off, so ignoring incoming operations. Object ref: %s, "
							"resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}

		bool bSomeObjectsWereMapped = false;
		TArray<GDK_PROPERTY(Property)*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);
		if (ShadowData.Num() == 0)
		{
			DependentChannel->ResetShadowData(RepLayout, ShadowData, ReplicatingObject);
		}

		ResolveObjectReferences(RepLayout, ReplicatingObject, *RepState, RepState->ReferenceMap, ShadowData.GetData(),
								(uint8*)ReplicatingObject, ReplicatingObject->GetClass()->GetPropertiesSize(), RepNotifies,
								bSomeObjectsWereMapped);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, RepLayout, RepState->ReferenceMap, ReplicatingObject);
			RepNotifyCalls.Push({ DependentChannel, ReplicatingObject, MoveTemp(RepNotifies) });
		}

		RepState->UnresolvedRefs.Remove(ObjectRef);
	}

	for (auto& RepNotify : RepNotifyCalls)
	{
		UE_LOG(LogSpatialReceiver, Verbose, TEXT("Resolved for target object %s"), *RepNotify.Object->GetName());
		RepNotify.Channel->PostReceiveSpatialUpdate(RepNotify.Object, RepNotify.Notifies, {});
	}
}

void USpatialReceiver::ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
											   FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
											   int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies,
											   bool& bOutSomeObjectsWereMapped)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogSpatialReceiver, Error, TEXT("ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"),
				   AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		FObjectReferences& ObjectReferences = It.Value();

		GDK_PROPERTY(Property)* Property = ObjectReferences.Property;

		// ParentIndex is -1 for handover properties
		bool bIsHandover = ObjectReferences.ParentIndex == -1;
		FRepParentCmd* Parent = ObjectReferences.ParentIndex >= 0 ? &RepLayout.Parents[ObjectReferences.ParentIndex] : nullptr;

		int32 StoredDataOffset = ObjectReferences.ShadowOffset;

		if (ObjectReferences.Array)
		{
			GDK_PROPERTY(ArrayProperty)* ArrayProperty = GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property);
			check(ArrayProperty != nullptr);

			if (!bIsHandover)
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			FScriptArray* StoredArray = bIsHandover ? nullptr : (FScriptArray*)(StoredData + StoredDataOffset);
			FScriptArray* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = Array->Num() * ArrayProperty->Inner->ElementSize;

			ResolveObjectReferences(RepLayout, ReplicatedObject, RepState, *ObjectReferences.Array,
									bIsHandover ? nullptr : (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset,
									RepNotifies, bOutSomeObjectsWereMapped);
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;
		FUnrealObjectRef SinglePropRef = FUnrealObjectRef::NULL_OBJECT_REF;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			FUnrealObjectRef& ObjectRef = *UnresolvedIt;

			bool bUnresolved = false;
			UObject* Object = FUnrealObjectRef::ToObjectPtr(ObjectRef, PackageMap, bUnresolved);
			if (!bUnresolved)
			{
				check(Object != nullptr);

				UE_LOG(LogSpatialReceiver, Verbose,
					   TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"),
					   AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
					SinglePropRef = ObjectRef;
				}

				UnresolvedIt.RemoveCurrent();

				bResolvedSomeRefs = true;
			}
		}

		if (bResolvedSomeRefs)
		{
			if (!bOutSomeObjectsWereMapped)
			{
				ReplicatedObject->PreNetReceive();
				bOutSomeObjectsWereMapped = true;
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			if (ObjectReferences.bSingleProp)
			{
				GDK_PROPERTY(ObjectPropertyBase)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
				ObjectReferences.MappedRefs.Add(SinglePropRef);
			}
			else if (ObjectReferences.bFastArrayProp)
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader ValueDataReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits,
													 NewMappedRefs, NewUnresolvedRefs);

				check(Property->IsA<GDK_PROPERTY(ArrayProperty)>());
				UScriptStruct* NetDeltaStruct = GetFastArraySerializerProperty(GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property));

				FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(NetDriver, ValueDataReader, ReplicatedObject, Parent->ArrayIndex,
																  Parent->Property, NetDeltaStruct);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}
			else
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader BitReader(PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits, NewMappedRefs,
											   NewUnresolvedRefs);
				check(Property->IsA<GDK_PROPERTY(StructProperty)>());

				bool bHasUnresolved = false;
				ReadStructProperty(BitReader, GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(Property), NetDriver, Data + AbsOffset,
								   bHasUnresolved);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent->RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + StoredDataOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent->Property);
				}
			}
		}
	}
}

void USpatialReceiver::OnHeartbeatComponentUpdate(const Worker_ComponentUpdateOp& Op)
{
	if (!NetDriver->IsServer())
	{
		// Clients can ignore Heartbeat component updates.
		return;
	}

	TWeakObjectPtr<USpatialNetConnection>* ConnectionPtr = AuthorityPlayerControllerConnectionMap.Find(Op.entity_id);
	if (ConnectionPtr == nullptr)
	{
		// Heartbeat component update on a PlayerController that this server does not have authority over.
		// TODO: Disable component interest for Heartbeat components this server doesn't care about - UNR-986
		return;
	}

	if (!ConnectionPtr->IsValid())
	{
		UE_LOG(LogSpatialReceiver, Warning,
			   TEXT("Received heartbeat component update after NetConnection has been cleaned up. PlayerController entity: %lld"),
			   Op.entity_id);
		AuthorityPlayerControllerConnectionMap.Remove(Op.entity_id);
		return;
	}

	USpatialNetConnection* NetConnection = ConnectionPtr->Get();

	Schema_Object* EventsObject = Schema_GetComponentUpdateEvents(Op.update.schema_type);
	uint32 EventCount = Schema_GetObjectCount(EventsObject, SpatialConstants::HEARTBEAT_EVENT_ID);
	if (EventCount > 0)
	{
		if (EventCount > 1)
		{
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("Received multiple heartbeat events in a single component update, entity %lld."),
				   Op.entity_id);
		}

		NetConnection->OnHeartbeat();
	}

	Schema_Object* FieldsObject = Schema_GetComponentUpdateFields(Op.update.schema_type);
	if (Schema_GetBoolCount(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID) > 0
		&& GetBoolFromSchema(FieldsObject, SpatialConstants::HEARTBEAT_CLIENT_HAS_QUIT_ID))
	{
		// Client has disconnected, let's clean up their connection.
		CloseClientConnection(NetConnection, Op.entity_id);
	}
}

void USpatialReceiver::CloseClientConnection(USpatialNetConnection* ClientConnection, Worker_EntityId PlayerControllerEntityId)
{
	ClientConnection->CleanUp();
	AuthorityPlayerControllerConnectionMap.Remove(PlayerControllerEntityId);
}

bool USpatialReceiver::NeedToLoadClass(const FString& ClassPath)
{
	UObject* ClassObject = FindObject<UClass>(nullptr, *ClassPath, false);
	if (ClassObject == nullptr)
	{
		return true;
	}

	FString PackagePath = GetPackagePath(ClassPath);
	FName PackagePathName = *PackagePath;

	// UNR-3320 The following test checks if the package is currently being processed in the async loading thread.
	// Without it, we could be using an object loaded in memory, but not completely ready to be used.
	// Looking through PackageMapClient's code, which handles asset async loading in Native unreal, checking
	// UPackage::IsFullyLoaded, or UObject::HasAnyInternalFlag(EInternalObjectFlag::AsyncLoading) should tell us if it is the case.
	// In practice, these tests are not enough to prevent using objects too early (symptom is RF_NeedPostLoad being set, and crash when
	// using them later). GetAsyncLoadPercentage will actually look through the async loading thread's UAsyncPackage maps to see if there
	// are any entries.
	// TODO : UNR-3374 This looks like an expensive check, but it does the job. We should investigate further
	// what is the issue with the other flags and why they do not give us reliable information.

	float Percentage = GetAsyncLoadPercentage(PackagePathName);
	if (Percentage != -1.0f)
	{
		UE_LOG(LogSpatialReceiver, Warning, TEXT("Class %s package is registered in async loading thread."), *ClassPath)
		return true;
	}

	return false;
}

FString USpatialReceiver::GetPackagePath(const FString& ClassPath)
{
	return FSoftObjectPath(ClassPath).GetLongPackageName();
}

void USpatialReceiver::MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref)
{
	if (TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref))
	{
		for (const FChannelObjectPair& ChannelObject : *RepStatesWithMappedRef)
		{
			if (USpatialActorChannel* Channel = ChannelObject.Key.Get())
			{
				if (FSpatialObjectRepState* RepState = Channel->ObjectReferenceMap.Find(ChannelObject.Value))
				{
					RepState->MoveMappedObjectToUnmapped(Ref);
				}
			}
		}
	}
}

void USpatialReceiver::RetireWhenAuthoritative(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup,
											   bool bNeedsTearOff)
{
	DeferredRetire DeferredObj = { EntityId, ActorClassId, bIsNetStartup, bNeedsTearOff };
	EntitiesToRetireOnAuthorityGain.Add(DeferredObj);
}

bool USpatialReceiver::IsEntityWaitingForAsyncLoad(Worker_EntityId Entity)
{
	return EntitiesWaitingForAsyncLoad.Contains(Entity);
}

void USpatialReceiver::QueueAddComponentOpForAsyncLoad(const Worker_AddComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.AddComponent(Op.entity_id, ComponentData::CreateCopy(Op.data.schema_type, Op.data.component_id));
}

void USpatialReceiver::QueueRemoveComponentOpForAsyncLoad(const Worker_RemoveComponentOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.RemoveComponent(Op.entity_id, Op.component_id);
}

void USpatialReceiver::QueueAuthorityOpForAsyncLoad(const Worker_ComponentSetAuthorityChangeOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	// todo UNR-4198 - This needs to be changed as it abuses the authority ops in a way that happens to work here.
	// It's fine in this case to not give a valid set of component data in the authority op as we don't try to parse the component
	// data when reading it. You couldn't create a view delta from this op list but it works here.
	AsyncLoadEntity.PendingOps.SetAuthority(Op.entity_id, Op.component_set_id, static_cast<Worker_Authority>(Op.authority), {});
}

void USpatialReceiver::QueueComponentUpdateOpForAsyncLoad(const Worker_ComponentUpdateOp& Op)
{
	EntityWaitingForAsyncLoad& AsyncLoadEntity = EntitiesWaitingForAsyncLoad.FindChecked(Op.entity_id);

	AsyncLoadEntity.PendingOps.UpdateComponent(Op.entity_id, ComponentUpdate::CreateCopy(Op.update.schema_type, Op.update.component_id));
}

namespace
{
FString GetObjectNameFromRepState(const FSpatialObjectRepState& RepState)
{
	if (UObject* Obj = RepState.GetChannelObjectPair().Value.Get())
	{
		return Obj->GetName();
	}
	return TEXT("<unknown>");
}
} // namespace

void USpatialReceiver::CleanupRepStateMap(FSpatialObjectRepState& RepState)
{
	for (const FUnrealObjectRef& Ref : RepState.ReferencedObj)
	{
		TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref);
		if (ensureMsgf(RepStatesWithMappedRef,
					   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
					   *GetObjectNameFromRepState(RepState)))
		{
			checkf(RepStatesWithMappedRef->Contains(RepState.GetChannelObjectPair()),
				   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
				   *GetObjectNameFromRepState(RepState));
			RepStatesWithMappedRef->Remove(RepState.GetChannelObjectPair());
			if (RepStatesWithMappedRef->Num() == 0)
			{
				GDK_ENSURE_NO_MODIFICATIONS(ObjectRefToRepStateUsageLock);
				ObjectRefToRepStateMap.Remove(Ref);
			}
		}
	}
}

bool USpatialReceiver::HasEntityBeenRequestedForDelete(Worker_EntityId EntityId)
{
	return EntitiesToRetireOnAuthorityGain.ContainsByPredicate([EntityId](const DeferredRetire& Retire) {
		return EntityId == Retire.EntityId;
	});
}

void USpatialReceiver::HandleDeferredEntityDeletion(const DeferredRetire& Retire)
{
	if (Retire.bNeedsTearOff)
	{
		Sender->SendActorTornOffUpdate(Retire.EntityId, Retire.ActorClassId);
		NetDriver->DelayedRetireEntity(Retire.EntityId, 1.0f, Retire.bIsNetStartupActor);
	}
	else
	{
		Sender->RetireEntity(Retire.EntityId, Retire.bIsNetStartupActor);
	}
}

void USpatialReceiver::HandleEntityDeletedAuthority(Worker_EntityId EntityId)
{
	int32 Index = EntitiesToRetireOnAuthorityGain.IndexOfByPredicate([EntityId](const DeferredRetire& Retire) {
		return Retire.EntityId == EntityId;
	});
	if (Index != INDEX_NONE)
	{
		HandleDeferredEntityDeletion(EntitiesToRetireOnAuthorityGain[Index]);
	}
}

bool USpatialReceiver::IsDynamicSubObject(AActor* Actor, uint32 SubObjectOffset)
{
	const FClassInfo& ActorClassInfo = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	return !ActorClassInfo.SubobjectInfo.Contains(SubObjectOffset);
}
