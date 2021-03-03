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

#include "Interop/CreateEntityHandler.h"
#include "Interop/ReserveEntityIdsHandler.h"

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

void CreateEntityHandler::AddRequest(Worker_RequestId RequestId, CreateEntityDelegate Handler)
{
	check(Handler.IsBound());
	Handlers.Add(RequestId, Handler);
}

void CreateEntityHandler::ProcessOps(const TArray<Worker_Op>& Ops)
{
	for (const Worker_Op& Op : Ops)
	{
		if (Op.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE)
		{
			SCOPE_CYCLE_COUNTER(STAT_ReceiverCreateEntityResponse);

			const Worker_CreateEntityResponseOp& EntityIdsOp = Op.op.create_entity_response;

			if (EntityIdsOp.status_code != WORKER_STATUS_CODE_SUCCESS)
			{
				UE_LOG(LogSpatialReceiver, Warning, TEXT("CreateEntity request failed: request id: %d, message: %s"),
					   EntityIdsOp.request_id, UTF8_TO_TCHAR(EntityIdsOp.message));
				return;
			}

			UE_LOG(LogSpatialReceiver, Verbose, TEXT("CreateEntity request succeeded: request id: %d, message: %s"), EntityIdsOp.request_id,
				   UTF8_TO_TCHAR(EntityIdsOp.message));

			const Worker_RequestId RequestId = EntityIdsOp.request_id;
			CreateEntityDelegate* Handler = Handlers.Find(RequestId);
			if (Handler != nullptr && ensure(Handler->IsBound()))
			{
				Handler->Execute(EntityIdsOp);
			}
			if (Handler != nullptr)
			{
				Handlers.Remove(RequestId);
			}
		}
	}
}

ClaimPartitionHandler::ClaimPartitionHandler(SpatialOSWorkerInterface& InConnection)
	: WorkerInterface(InConnection)
{
}

void ClaimPartitionHandler::ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim)
{
	UE_LOG(LogTemp, Log,
		   TEXT("SendClaimPartitionRequest. Worker: %s, SystemWorkerEntityId: %lld. "
				"PartitionId: %lld"),
		   TEXT("UNK") /* *Connection->GetWorkerId()*/, SystemEntityId, PartitionToClaim);

	Worker_CommandRequest CommandRequest = Worker::CreateClaimPartitionRequest(PartitionToClaim);
	const Worker_RequestId ClaimEntityRequestId =
		WorkerInterface.SendCommandRequest(SystemEntityId, &CommandRequest, RETRY_UNTIL_COMPLETE, {});
	ClaimPartitionRequestIds.Add(ClaimEntityRequestId, PartitionToClaim);
}

void ClaimPartitionHandler::ProcessOps(const TArray<Worker_Op>& Ops)
{
	for (const Worker_Op& Op : Ops)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE)
		{
			const Worker_CommandResponseOp& CommandResponse = Op.op.command_response;
			Worker_PartitionId ClaimedPartitionId = SpatialConstants::INVALID_PARTITION_ID;
			const bool bIsRequestHandled = ClaimPartitionRequestIds.RemoveAndCopyValue(CommandResponse.request_id, ClaimedPartitionId);
			if (bIsRequestHandled)
			{
				ensure(CommandResponse.response.component_id == SpatialConstants::WORKER_COMPONENT_ID);
				ensureMsgf(CommandResponse.status_code == WORKER_STATUS_CODE_SUCCESS,
						   TEXT("Claim partition request for partition %lld finished, SDK returned code %d [%s]"), ClaimedPartitionId,
						   (int)CommandResponse.status_code, UTF8_TO_TCHAR(CommandResponse.message));
			}
		}
	}
}

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
	if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID
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
	if (ComponentId == SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID)
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

void USpatialReceiver::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<FReliableRPCForRetry> ReliableRPC)
{
	PendingReliableRPCs.Add(RequestId, ReliableRPC);
}

void USpatialReceiver::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, MoveTemp(Delegate));
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

	if (Channel.HackPendingCreateEntityRequestIds.Num() > 0)
	{
		return true;
	}

	return false;
}
