// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialSender.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/NetworkProfiler.h"
#include "Runtime/Launch/Resources/Version.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialLoadBalanceEnforcer.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/CrossServerEndpoint.h"
#include "Schema/Interest.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/InterestFactory.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialDebuggerSystem.h"
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("Sender SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
	EventTracer = InEventTracer;
}

void USpatialSender::CreateServerWorkerEntity()
{
	RetryServerWorkerEntityCreation(PackageMap->AllocateEntityId(), 1);
}

// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
void USpatialSender::RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter)
{
	check(NetDriver != nullptr);
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateComponentData());
	Components.Add(ServerWorker(Connection->GetWorkerId(), false, Connection->GetWorkerSystemEntityId()).CreateServerWorkerData());

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, EntityId);
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	if (Settings->CrossServerRPCImplementation == ECrossServerRPCImplementation::RoutingWorker)
	{
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID));
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSSSERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID));
	}
	check(NetDriver != nullptr);

	// The load balance strategy won't be set up at this point, but we call this function again later when it is ready in
	// order to set the interest of the server worker according to the strategy.
	Components.Add(NetDriver->InterestFactory->CreateServerWorkerInterest(NetDriver->LoadBalanceStrategy).CreateComponentData());

	// GDK known entities completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	const Worker_RequestId RequestId = Connection->SendCreateEntityRequest(MoveTemp(Components), &EntityId, RETRY_UNTIL_COMPLETE);

	CreateEntityDelegate OnCreateWorkerEntityResponse;
	OnCreateWorkerEntityResponse.BindLambda(
		[WeakSender = TWeakObjectPtr<USpatialSender>(this), EntityId, AttemptCounter](const Worker_CreateEntityResponseOp& Op) {
			if (!WeakSender.IsValid())
			{
				return;
			}
			USpatialSender* Sender = WeakSender.Get();

			if (Op.status_code == WORKER_STATUS_CODE_SUCCESS)
			{
				Sender->NetDriver->WorkerEntityId = Op.entity_id;

				// We claim each server worker entity as a partition for server worker interest. This is necessary for getting
				// interest in the VirtualWorkerTranslator component.
				Sender->SendClaimPartitionRequest(Sender->NetDriver->Connection->GetWorkerSystemEntityId(), Op.entity_id);

				return;
			}

			// Given the nature of commands, it's possible we have multiple create commands in flight at once. If a command fails where
			// we've already set the worker entity ID locally, this means we already successfully create the entity, so nothing needs doing.
			if (Op.status_code != WORKER_STATUS_CODE_SUCCESS && Sender->NetDriver->WorkerEntityId != SpatialConstants::INVALID_ENTITY_ID)
			{
				return;
			}

			if (Op.status_code != WORKER_STATUS_CODE_TIMEOUT)
			{
				UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request failed: \"%s\""), UTF8_TO_TCHAR(Op.message));
				return;
			}

			if (AttemptCounter == SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
			{
				UE_LOG(LogSpatialSender, Error, TEXT("Worker entity creation request timed out too many times. (%u attempts)"),
					   SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS);
				return;
			}

			UE_LOG(LogSpatialSender, Warning, TEXT("Worker entity creation request timed out and will retry."));
			FTimerHandle RetryTimer;
			Sender->TimerManager->SetTimer(
				RetryTimer,
				[WeakSender, EntityId, AttemptCounter]() {
					if (USpatialSender* SpatialSender = WeakSender.Get())
					{
						SpatialSender->RetryServerWorkerEntityCreation(EntityId, AttemptCounter + 1);
					}
				},
				SpatialConstants::GetCommandRetryWaitTimeSeconds(AttemptCounter), false);
		});

	Receiver->AddCreateEntityDelegate(RequestId, MoveTemp(OnCreateWorkerEntityResponse));
}

bool USpatialSender::ValidateOrExit_IsSupportedClass(const FString& PathName)
{
	// Level blueprint classes could have a PIE prefix, this will remove it.
	FString RemappedPathName = PathName;
#if ENGINE_MINOR_VERSION >= 26
	GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), RemappedPathName, false /*bIsReading*/);
#else
	GEngine->NetworkRemapPath(NetDriver, RemappedPathName, false /*bIsReading*/);
#endif

	return ClassInfoManager->ValidateOrExit_IsSupportedClass(RemappedPathName);
}

void USpatialSender::SendClaimPartitionRequest(Worker_EntityId SystemWorkerEntityId, Worker_PartitionId PartitionId) const
{
	UE_LOG(LogSpatialSender, Log,
		   TEXT("SendClaimPartitionRequest. Worker: %s, SystemWorkerEntityId: %lld. "
				"PartitionId: %lld"),
		   *Connection->GetWorkerId(), SystemWorkerEntityId, PartitionId);
	Worker_CommandRequest CommandRequest = Worker::CreateClaimPartitionRequest(PartitionId);
	const Worker_RequestId RequestId = Connection->SendCommandRequest(SystemWorkerEntityId, &CommandRequest, RETRY_UNTIL_COMPLETE, {});
	Receiver->PendingPartitionAssignments.Add(RequestId, PartitionId);
}

void USpatialSender::UpdatePartitionEntityInterestAndPosition()
{
	check(Connection != nullptr);
	check(NetDriver != nullptr);
	check(NetDriver->VirtualWorkerTranslator != nullptr
		  && NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId() != SpatialConstants::INVALID_ENTITY_ID);
	check(NetDriver->LoadBalanceStrategy != nullptr && NetDriver->LoadBalanceStrategy->IsReady());

	Worker_PartitionId PartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();
	VirtualWorkerId VirtualId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();

	// Update the interest. If it's ready and not null, also adds interest according to the load balancing strategy.
	FWorkerComponentUpdate InterestUpdate =
		NetDriver->InterestFactory
			->CreatePartitionInterest(NetDriver->LoadBalanceStrategy, VirtualId, NetDriver->DebugCtx != nullptr /*bDebug*/)
			.CreateInterestUpdate();

	Connection->SendComponentUpdate(PartitionId, &InterestUpdate);

	// Also update the position of the partition entity to the center of the load balancing region.
	FWorkerComponentUpdate Update =
		Position::CreatePositionUpdate(Coordinates::FromFVector(NetDriver->LoadBalanceStrategy->GetWorkerEntityPosition()));
	Connection->SendComponentUpdate(PartitionId, &Update);
}

void USpatialSender::SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId) const
{
	const Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(&Actor);
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

	TOptional<AuthorityIntent> AuthorityIntentComponent = NetDriver->Connection->GetCoordinator().GetComponent<AuthorityIntent>(EntityId);
	check(AuthorityIntentComponent.IsSet());
	checkf(AuthorityIntentComponent->VirtualWorkerId != NewAuthoritativeVirtualWorkerId,
		   TEXT("Attempted to update AuthorityIntent twice to the same value. Actor: %s. Entity ID: %lld. Virtual worker: '%d'"),
		   *GetNameSafe(&Actor), EntityId, NewAuthoritativeVirtualWorkerId);

	AuthorityIntentComponent->VirtualWorkerId = NewAuthoritativeVirtualWorkerId;
	UE_LOG(LogSpatialSender, Log,
		   TEXT("(%s) Sending AuthorityIntent update for entity id %d. Virtual worker '%d' should become authoritative over %s"),
		   *NetDriver->Connection->GetWorkerId(), EntityId, NewAuthoritativeVirtualWorkerId, *GetNameSafe(&Actor));

	FWorkerComponentUpdate Update = AuthorityIntentComponent->CreateAuthorityIntentUpdate();

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateAuthorityIntentUpdate(NewAuthoritativeVirtualWorkerId, &Actor));
	}

	Connection->SendComponentUpdate(EntityId, &Update, SpanId);

	// Notify the enforcer directly on the worker that sends the component update, as the update will short circuit.
	// This should always happen with USLB.
	NetDriver->LoadBalanceEnforcer->ShortCircuitMaybeRefreshAuthorityDelegation(EntityId);

	if (NetDriver->SpatialDebuggerSystem.IsValid())
	{
		NetDriver->SpatialDebuggerSystem->ActorAuthorityIntentChanged(EntityId, NewAuthoritativeVirtualWorkerId);
	}
}

void USpatialSender::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response, const FSpatialGDKSpanId& CauseSpanId)
{
	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true),
										 /* Causes */ CauseSpanId.GetConstId(), /* NumCauses */ 1);
	}

	Connection->SendCommandResponse(RequestId, &Response, SpanId);
}

void USpatialSender::SendEmptyCommandResponse(Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, Worker_RequestId RequestId,
											  const FSpatialGDKSpanId& CauseSpanId)
{
	Worker_CommandResponse Response = {};
	Response.component_id = ComponentId;
	Response.command_index = CommandIndex;
	Response.schema_type = Schema_CreateCommandResponse();

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId =
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true), CauseSpanId.GetConstId(), 1);
	}

	Connection->SendCommandResponse(RequestId, &Response, SpanId);
}

void USpatialSender::SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& CauseSpanId)
{
	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId =
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, false), CauseSpanId.GetConstId(), 1);
	}

	Connection->SendCommandFailure(RequestId, Message, SpanId);
}
