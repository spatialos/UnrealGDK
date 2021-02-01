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
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
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
#include "Utils/SpatialLatencyTracer.h"
#include "Utils/SpatialMetrics.h"
#include "Utils/SpatialStatics.h"

DEFINE_LOG_CATEGORY(LogSpatialSender);

using namespace SpatialGDK;

DECLARE_CYCLE_STAT(TEXT("Sender SendComponentUpdates"), STAT_SpatialSenderSendComponentUpdates, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender ResetOutgoingUpdate"), STAT_SpatialSenderResetOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender QueueOutgoingUpdate"), STAT_SpatialSenderQueueOutgoingUpdate, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender UpdateInterestComponent"), STAT_SpatialSenderUpdateInterestComponent, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Sender SendRPC"), STAT_SpatialSenderSendRPC, STATGROUP_SpatialNet);

void USpatialSender::Init(USpatialNetDriver* InNetDriver, FTimerManager* InTimerManager, SpatialGDK::SpatialRPCService* InRPCService,
						  SpatialGDK::SpatialEventTracer* InEventTracer)
{
	NetDriver = InNetDriver;
	StaticComponentView = InNetDriver->StaticComponentView;
	Connection = InNetDriver->Connection;
	Receiver = InNetDriver->Receiver;
	PackageMap = InNetDriver->PackageMap;
	ClassInfoManager = InNetDriver->ClassInfoManager;
	TimerManager = InTimerManager;
	RPCService = InRPCService;
	EventTracer = InEventTracer;

	OutgoingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateUObject(this, &USpatialSender::SendRPC));

	// Attempt to send RPCs that might have been queued while waiting for authority over entities this worker created.
	if (GetDefault<USpatialGDKSettings>()->QueuedOutgoingRPCRetryTime > 0.0f)
	{
		PeriodicallyProcessOutgoingRPCs();
	}
}

Worker_RequestId USpatialSender::CreateEntity(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	EntityFactory DataFactory(NetDriver, PackageMap, ClassInfoManager, RPCService);
	TArray<FWorkerComponentData> ComponentDatas = DataFactory.CreateEntityComponents(Channel, OutBytesWritten);

	// If the Actor was loaded rather than dynamically spawned, associate it with its owning sublevel.
	ComponentDatas.Add(CreateLevelComponentData(Channel->Actor));

	Worker_EntityId EntityId = Channel->GetEntityId();

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCreateEntity(Channel->Actor, EntityId));
	}

	Worker_RequestId CreateEntityRequestId =
		Connection->SendCreateEntityRequest(MoveTemp(ComponentDatas), &EntityId, RETRY_UNTIL_COMPLETE, SpanId);

	return CreateEntityRequestId;
}

void USpatialSender::PeriodicallyProcessOutgoingRPCs()
{
	FTimerHandle Timer;
	TimerManager->SetTimer(
		Timer,
		[WeakThis = TWeakObjectPtr<USpatialSender>(this)]() {
			if (USpatialSender* SpatialSender = WeakThis.Get())
			{
				SpatialSender->OutgoingRPCs.ProcessRPCs();
			}
		},
		GetDefault<USpatialGDKSettings>()->QueuedOutgoingRPCRetryTime, true);
}

void USpatialSender::SendAddComponentForSubobject(USpatialActorChannel* Channel, UObject* Subobject, const FClassInfo& SubobjectInfo,
												  uint32& OutBytesWritten)
{
	FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
	FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

	ComponentFactory DataFactory(false, NetDriver, USpatialLatencyTracer::GetTracer(Subobject));

	TArray<FWorkerComponentData> SubobjectDatas =
		DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges, OutBytesWritten);
	SendAddComponents(Channel->GetEntityId(), SubobjectDatas);

	Channel->PendingDynamicSubobjects.Remove(TWeakObjectPtr<UObject>(Subobject));
}

void USpatialSender::SendAddComponents(Worker_EntityId EntityId, TArray<FWorkerComponentData> ComponentDatas)
{
	if (ComponentDatas.Num() == 0)
	{
		return;
	}

	for (FWorkerComponentData& ComponentData : ComponentDatas)
	{
		Connection->SendAddComponent(EntityId, &ComponentData);
	}
}

void USpatialSender::SendRemoveComponentForClassInfo(Worker_EntityId EntityId, const FClassInfo& Info)
{
	TArray<Worker_ComponentId> ComponentsToRemove;
	ComponentsToRemove.Reserve(SCHEMA_Count);
	for (Worker_ComponentId SubobjectComponentId : Info.SchemaComponents)
	{
		if (SubobjectComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			ComponentsToRemove.Add(SubobjectComponentId);
		}
	}

	SendRemoveComponents(EntityId, ComponentsToRemove);

	PackageMap->RemoveSubobject(FUnrealObjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]));
}

void USpatialSender::SendRemoveComponents(Worker_EntityId EntityId, TArray<Worker_ComponentId> ComponentIds)
{
	for (auto ComponentId : ComponentIds)
	{
		Connection->SendRemoveComponent(EntityId, ComponentId);
	}
}

void USpatialSender::CreateServerWorkerEntity()
{
	RetryServerWorkerEntityCreation(PackageMap->AllocateEntityId(), 1);
}

// Creates an entity authoritative on this server worker, ensuring it will be able to receive updates for the GSM.
void USpatialSender::RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter)
{
	check(NetDriver != nullptr);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Metadata(FString::Format(TEXT("WorkerEntity:{0}"), { Connection->GetWorkerId() })).CreateComponentData());
	Components.Add(ServerWorker(Connection->GetWorkerId(), false, Connection->GetWorkerSystemEntityId()).CreateServerWorkerData());

	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, EntityId);
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

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
				Sender->SendClaimPartitionRequest(WeakSender->NetDriver->Connection->GetWorkerSystemEntityId(), Op.entity_id);

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

void USpatialSender::ClearPendingRPCs(const Worker_EntityId EntityId)
{
	OutgoingRPCs.DropForEntity(EntityId);
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

void USpatialSender::FlushRPCService()
{
	if (RPCService != nullptr)
	{
		RPCService->PushOverflowedRPCs();

		TArray<SpatialRPCService::UpdateToSend> RPCs = RPCService->GetRPCsAndAcksToSend();
		for (SpatialRPCService::UpdateToSend& Update : RPCs)
		{
			Connection->SendComponentUpdate(Update.EntityId, &Update.Update, Update.SpanId);
		}

		if (RPCs.Num() && GetDefault<USpatialGDKSettings>()->bWorkerFlushAfterOutgoingNetworkOp)
		{
			Connection->Flush();
		}
	}
}

RPCPayload USpatialSender::CreateRPCPayloadFromParams(UObject* TargetObject, const FUnrealObjectRef& TargetObjectRef, UFunction* Function,
													  ERPCType Type, void* Params)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	FSpatialNetBitWriter PayloadWriter = PackRPCDataToSpatialNetBitWriter(Function, Params);

	TOptional<uint64> Id;
	if (Type == ERPCType::CrossServer)
	{
		Id = FMath::RandRange(static_cast<int64>(0), static_cast<int64>(INT64_MAX));
	}

#if TRACE_LIB_ACTIVE
	return RPCPayload(TargetObjectRef.Offset, RPCInfo.Index, Id, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()),
					  USpatialLatencyTracer::GetTracer(TargetObject)->RetrievePendingTrace(TargetObject, Function));
#else
	return RPCPayload(TargetObjectRef.Offset, RPCInfo.Index, Id, TArray<uint8>(PayloadWriter.GetData(), PayloadWriter.GetNumBytes()));
#endif
}

void USpatialSender::SendInterestBucketComponentChange(const Worker_EntityId EntityId, const Worker_ComponentId OldComponent,
													   const Worker_ComponentId NewComponent)
{
	if (OldComponent != SpatialConstants::INVALID_COMPONENT_ID)
	{
		// No loopback, so simulate the operations locally.
		Worker_RemoveComponentOp RemoveOp{};
		RemoveOp.entity_id = EntityId;
		RemoveOp.component_id = OldComponent;
		StaticComponentView->OnRemoveComponent(RemoveOp);

		SendRemoveComponents(EntityId, { OldComponent });
	}

	if (NewComponent != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Worker_AddComponentOp AddOp{};
		AddOp.entity_id = EntityId;
		AddOp.data.component_id = NewComponent;
		AddOp.data.schema_type = nullptr;
		AddOp.data.user_handle = nullptr;

		StaticComponentView->OnAddComponent(AddOp);

		SendAddComponents(EntityId, { ComponentFactory::CreateEmptyComponentData(NewComponent) });
	}
}

void USpatialSender::SendActorTornOffUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	FWorkerComponentUpdate ComponentUpdate = {};

	ComponentUpdate.component_id = ComponentId;
	ComponentUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

	Schema_AddBool(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID, 1);

	Connection->SendComponentUpdate(EntityId, &ComponentUpdate);
}

void USpatialSender::SendPositionUpdate(Worker_EntityId EntityId, const FVector& Location)
{
#if !UE_BUILD_SHIPPING
	if (!NetDriver->HasServerAuthority(EntityId))
	{
		UE_LOG(LogSpatialSender, Verbose,
			   TEXT("Trying to send Position component update but don't have authority! Update will not be sent. Entity: %lld"), EntityId);
		return;
	}
#endif

	FWorkerComponentUpdate Update = Position::CreatePositionUpdate(Coordinates::FromFVector(Location));
	Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::SendAuthorityIntentUpdate(const AActor& Actor, VirtualWorkerId NewAuthoritativeVirtualWorkerId) const
{
	const Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(&Actor);
	check(EntityId != SpatialConstants::INVALID_ENTITY_ID);

	AuthorityIntent* AuthorityIntentComponent = StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	check(AuthorityIntentComponent != nullptr);
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

	if (NetDriver->SpatialDebugger != nullptr)
	{
		NetDriver->SpatialDebugger->ActorAuthorityIntentChanged(EntityId, NewAuthoritativeVirtualWorkerId);
	}
}

FRPCErrorInfo USpatialSender::SendRPC(const FPendingRPCParams& Params)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderSendRPC);

	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return FRPCErrorInfo{ nullptr, nullptr, ERPCResult::UnresolvedTargetObject, ERPCQueueProcessResult::DropEntireQueue };
	}
	UObject* TargetObject = TargetObjectWeakPtr.Get();

	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, nullptr, ERPCResult::MissingFunctionInfo, ERPCQueueProcessResult::ContinueProcessing };
	}

	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(TargetObject);
	if (Channel == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::NoActorChannel, ERPCQueueProcessResult::DropEntireQueue };
	}

	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	checkf(RPCService != nullptr, TEXT("RPCService is assumed to be valid."));
	if (SendRingBufferedRPC(TargetObject, Function, Params.Payload, Channel, Params.ObjectRef, Params.SpanId))
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
	}
	else
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::RPCServiceFailure };
	}
}

bool USpatialSender::SendRingBufferedRPC(UObject* TargetObject, UFunction* Function, const SpatialGDK::RPCPayload& Payload,
										 USpatialActorChannel* Channel, const FUnrealObjectRef& TargetObjectRef,
										 const FSpatialGDKSpanId& SpanId)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const EPushRPCResult Result =
		RPCService->PushRPC(TargetObjectRef.Entity, RPCInfo.Type, Payload, Channel->bCreatedEntity, TargetObject, Function, SpanId);

	if (Result == EPushRPCResult::Success)
	{
		FlushRPCService();
	}

#if !UE_BUILD_SHIPPING
	if (Result == EPushRPCResult::Success || Result == EPushRPCResult::QueueOverflowed)
	{
		TrackRPC(Channel->Actor, Function, Payload, RPCInfo.Type);
	}
#endif // !UE_BUILD_SHIPPING

	switch (Result)
	{
	case EPushRPCResult::QueueOverflowed:
		UE_LOG(LogSpatialSender, Log,
			   TEXT("USpatialSender::SendRingBufferedRPC: Ring buffer queue overflowed, queuing RPC locally. Actor: %s, entity: %lld, "
					"function: %s"),
			   *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::DropOverflowed:
		UE_LOG(
			LogSpatialSender, Log,
			TEXT("USpatialSender::SendRingBufferedRPC: Ring buffer queue overflowed, dropping RPC. Actor: %s, entity: %lld, function: %s"),
			*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::HasAckAuthority:
		UE_LOG(LogSpatialSender, Warning,
			   TEXT("USpatialSender::SendRingBufferedRPC: Worker has authority over ack component for RPC it is sending. RPC will not be "
					"sent. Actor: %s, entity: %lld, function: %s"),
			   *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::NoRingBufferAuthority:
		// TODO: Change engine logic that calls Client RPCs from non-auth servers and change this to error. UNR-2517
		UE_LOG(LogSpatialSender, Log,
			   TEXT("USpatialSender::SendRingBufferedRPC: Failed to send RPC because the worker does not have authority over ring buffer "
					"component. Actor: %s, entity: %lld, function: %s"),
			   *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return true;
	case EPushRPCResult::EntityBeingCreated:
		UE_LOG(LogSpatialSender, Log,
			   TEXT("USpatialSender::SendRingBufferedRPC: RPC was called between entity creation and initial authority gain, so it will be "
					"queued. Actor: %s, entity: %lld, function: %s"),
			   *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
		return false;
	default:
		return true;
	}
}

#if !UE_BUILD_SHIPPING
void USpatialSender::TrackRPC(AActor* Actor, UFunction* Function, const RPCPayload& Payload, const ERPCType RPCType)
{
	NETWORK_PROFILER(GNetworkProfiler.TrackSendRPC(Actor, Function, 0, Payload.CountDataBits(), 0, NetDriver->GetSpatialOSNetConnection()));
	NetDriver->SpatialMetrics->TrackSentRPC(Function, RPCType, Payload.PayloadData.Num());
}
#endif

void USpatialSender::RegisterChannelForPositionUpdate(USpatialActorChannel* Channel)
{
	ChannelsToUpdatePosition.Add(Channel);
}

void USpatialSender::ProcessPositionUpdates()
{
	for (auto& Channel : ChannelsToUpdatePosition)
	{
		if (Channel.IsValid())
		{
			Channel->UpdateSpatialPosition();
		}
	}

	ChannelsToUpdatePosition.Empty();
}

void USpatialSender::SendCreateEntityRequest(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	UE_LOG(LogSpatialSender, Log, TEXT("Sending create entity request for %s with EntityId %lld, HasAuthority: %d"),
		   *Channel->Actor->GetName(), Channel->GetEntityId(), Channel->Actor->HasAuthority());

	Worker_RequestId RequestId = CreateEntity(Channel, OutBytesWritten);

	Receiver->AddPendingActorRequest(RequestId, Channel);
}

void USpatialSender::ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload&& InPayload)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = PackageMap->GetObjectFromUnrealObjectRef(InTargetObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);
	UFunction* Function = ClassInfo.RPCs[InPayload.Index];
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);

	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreatePushRPC(TargetObject, Function),
										 EventTracer->GetFromStack().GetConstId(), 1);
	}

	OutgoingRPCs.ProcessOrQueueRPC(InTargetObjectRef, RPCInfo.Type, MoveTemp(InPayload), SpanId);

	// Try to send all pending RPCs unconditionally
	OutgoingRPCs.ProcessRPCs();
}

FSpatialNetBitWriter USpatialSender::PackRPCDataToSpatialNetBitWriter(UFunction* Function, void* Parameters) const
{
	FSpatialNetBitWriter PayloadWriter(PackageMap);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_SendPropertiesForRPC(*RepLayout, PayloadWriter, Parameters);

	return PayloadWriter;
}

void USpatialSender::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse& Response, const FSpatialGDKSpanId& CauseSpanId)
{
	FSpatialGDKSpanId SpanId;
	if (EventTracer != nullptr)
	{
		SpanId =
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, true), CauseSpanId.GetConstId(), 1);
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

void USpatialSender::UpdateInterestComponent(AActor* Actor)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialSenderUpdateInterestComponent);

	Worker_EntityId EntityId = PackageMap->GetEntityIdFromObject(Actor);
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialSender, Verbose, TEXT("Attempted to update interest for non replicated actor: %s"), *GetNameSafe(Actor));
		return;
	}

	FWorkerComponentUpdate Update =
		NetDriver->InterestFactory->CreateInterestUpdate(Actor, ClassInfoManager->GetOrCreateClassInfoByObject(Actor), EntityId);

	Connection->SendComponentUpdate(EntityId, &Update);
}

void USpatialSender::RetireEntity(const Worker_EntityId EntityId, bool bIsNetStartupActor)
{
	if (bIsNetStartupActor)
	{
		NetDriver->ActorSystem->RemoveActor(EntityId);
		// In the case that this is a startup actor, we won't actually delete the entity in SpatialOS.  Instead we'll Tombstone it.
		if (!StaticComponentView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
		{
			UE_LOG(LogSpatialSender, Log, TEXT("Adding tombstone to entity: %lld"), EntityId);
			AddTombstoneToEntity(EntityId);
		}
		else
		{
			UE_LOG(LogSpatialSender, Verbose, TEXT("RetireEntity called on already retired entity: %lld"), EntityId);
		}
	}
	else
	{
		// Actor no longer guaranteed to be in package map, but still useful for additional logging info
		AActor* Actor = Cast<AActor>(PackageMap->GetObjectFromEntityId(EntityId));

		UE_LOG(LogSpatialSender, Log, TEXT("Sending delete entity request for %s with EntityId %lld, HasAuthority: %d"),
			   *GetPathNameSafe(Actor), EntityId, Actor != nullptr ? Actor->HasAuthority() : false);

		if (EventTracer != nullptr)
		{
			FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendRetireEntity(Actor, EntityId));
		}

		Connection->SendDeleteEntityRequest(EntityId, RETRY_UNTIL_COMPLETE);
	}
}
