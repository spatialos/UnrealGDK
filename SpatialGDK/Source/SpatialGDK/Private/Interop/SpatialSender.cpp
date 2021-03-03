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
	SubView = &NetDriver->Connection->GetCoordinator().CreateSubView(SpatialConstants::INVALID_QUERY_TAG, FSubView::NoFilter,
																	 FSubView::NoDispatcherCallbacks);
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

void USpatialSender::Advance()
{
	CreateEntityHandler.ProcessOps(*SubView->GetViewDelta().WorkerMessages);
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
		const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

		RPCService->PushOverflowedRPCs();

		TArray<SpatialRPCService::UpdateToSend> RPCs = RPCService->GetRPCsAndAcksToSend();

		for (SpatialRPCService::UpdateToSend& Update : RPCs)
		{
			Connection->SendComponentUpdate(Update.EntityId, &Update.Update, Update.SpanId);
		}

		if (RPCs.Num() && Settings->bWorkerFlushAfterOutgoingNetworkOp)
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

	if (NetDriver->SpatialDebuggerSystem.IsValid())
	{
		NetDriver->SpatialDebuggerSystem->ActorAuthorityIntentChanged(EntityId, NewAuthoritativeVirtualWorkerId);
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
	if (RPCInfo.Type == ERPCType::CrossServer)
	{
		if (SendCrossServerRPC(TargetObject, Params.SenderRPCInfo, Function, Params.Payload, Channel, Params.ObjectRef))
		{
			return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
		}
		else
		{
			return FRPCErrorInfo{ TargetObject, Function, ERPCResult::RPCServiceFailure };
		}
	}

	if (SendRingBufferedRPC(TargetObject, RPCSender(), Function, Params.Payload, Channel, Params.ObjectRef, Params.SpanId))
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::Success };
	}
	else
	{
		return FRPCErrorInfo{ TargetObject, Function, ERPCResult::RPCServiceFailure };
	}
}

bool USpatialSender::SendCrossServerRPC(UObject* TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
										const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel,
										const FUnrealObjectRef& TargetObjectRef)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	const bool bHasValidSender = Sender.Entity != SpatialConstants::INVALID_ENTITY_ID;

	check(Settings->CrossServerRPCImplementation == ECrossServerRPCImplementation::RoutingWorker);
	if (bHasValidSender)
	{
		return SendRingBufferedRPC(TargetObject, Sender, Function, Payload, Channel, TargetObjectRef, {});
	}

	return false;
}

bool USpatialSender::SendRingBufferedRPC(UObject* TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
										 const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel,
										 const FUnrealObjectRef& TargetObjectRef, const FSpatialGDKSpanId& SpanId)
{
	const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const EPushRPCResult Result =
		RPCService->PushRPC(TargetObjectRef.Entity, Sender, RPCInfo.Type, Payload, Channel->bCreatedEntity, TargetObject, Function, SpanId);

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

void USpatialSender::ProcessOrQueueOutgoingRPC(const FUnrealObjectRef& InTargetObjectRef, const SpatialGDK::RPCSender& InSenderInfo,
											   RPCPayload&& InPayload)
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
										 /* Causes */ EventTracer->GetFromStack().GetConstId(), /* NumCauses */ 1);
	}

	OutgoingRPCs.ProcessOrQueueRPC(InTargetObjectRef, InSenderInfo, RPCInfo.Type, MoveTemp(InPayload), SpanId);

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
