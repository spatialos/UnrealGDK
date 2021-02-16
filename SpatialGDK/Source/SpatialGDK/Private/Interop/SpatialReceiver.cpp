// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialReceiver.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialSender.h"
#include "Schema/MigrationDiagnostic.h"
#include "SpatialConstants.h"
#include "Utils/ErrorCodeRemapping.h"
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

void USpatialReceiver::Init(USpatialNetDriver* InNetDriver, SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	Sender = InNetDriver->Sender;
	PackageMap = InNetDriver->PackageMap;
	EventTracer = InEventTracer;
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

	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID
		&& CommandIndex == SpatialConstants::PLAYER_SPAWNER_SPAWN_PLAYER_COMMAND_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnRequestOnServer(CommandRequestOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SPAWN_PLAYER_COMMAND"), RequestId),
									/* Causes */ Op.span_id, /* NumCauses */ 1);
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
				/* Causes */ Op.span_id, /* NumCauses */ 1);
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
				FSpatialTraceEventBuilder::CreateReceiveCommandRequest(TEXT("SHUTDOWN_MULTI_PROCESS_REQUEST"), RequestId),
				/* Causes */ Op.span_id, /* NumCauses */ 1);
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
	const Worker_CommandResponse& CommandResponse = CommandResponseOp.response;
	const Worker_ComponentId ComponentId = CommandResponse.component_id;
	const Worker_RequestId RequestId = CommandResponseOp.request_id;

	SCOPE_CYCLE_COUNTER(STAT_ReceiverCommandResponse);
	if (ComponentId == SpatialConstants::PLAYER_SPAWNER_COMPONENT_ID)
	{
		NetDriver->PlayerSpawner->ReceivePlayerSpawnResponseOnClient(CommandResponseOp);

		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SPAWN_PLAYER_COMMAND"), RequestId),
									/* Causes */ EventTracer->GetAndConsumeSpanForRequestId(CommandResponseOp.request_id).GetConstId(),
									/* NumCauses */ 1);
		}

		return;
	}
	else if (ComponentId == SpatialConstants::SERVER_WORKER_COMPONENT_ID)
	{
		if (EventTracer != nullptr)
		{
			EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateReceiveCommandResponse(TEXT("SERVER_WORKER_FORWARD_SPAWN_REQUEST_COMMAND"), RequestId),
				/* Causes */ EventTracer->GetAndConsumeSpanForRequestId(CommandResponseOp.request_id).GetConstId(), 1);
		}
		NetDriver->PlayerSpawner->ReceiveForwardPlayerSpawnResponse(CommandResponseOp);
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
		const Worker_EntityId EntityId = Schema_GetInt64(ResponseObject, SpatialConstants::MIGRATION_DIAGNOSTIC_ENTITY_ID);
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
	if (Op.request_id < 0)
	{
		// Invalid request id that will not be in PendingPartitionAssignments
		return;
	}

	Worker_PartitionId PartitionId;
	if (!PendingPartitionAssignments.RemoveAndCopyValue(Op.request_id, PartitionId))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslationManager, Log,
			   TEXT("Could not find request id in PendingPartitionAssignments. Request Id: %d"), Op.request_id);
		return;
	}

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
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCreateEntitySuccess(Channel->Actor, EntityId),
									/* Causes */ Op.span_id, /* NumCauses */ 1);
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
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(Message), /* Causes */ Op.span_id, /* NumCauses */ 1);
		}
	}
	else if (EventTracer != nullptr)
	{
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateGenericMessage(TEXT("Create entity response unknown error")),
								/* Causes */ Op.span_id,
								/* NumCauses */ 1);
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
