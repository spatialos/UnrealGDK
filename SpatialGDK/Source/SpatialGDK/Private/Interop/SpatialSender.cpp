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

	bool USpatialSender::SendCrossServerRPC(UObject * TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
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

	bool USpatialSender::SendRingBufferedRPC(UObject * TargetObject, const SpatialGDK::RPCSender& Sender, UFunction* Function,
											 const SpatialGDK::RPCPayload& Payload, USpatialActorChannel* Channel,
											 const FUnrealObjectRef& TargetObjectRef, const FSpatialGDKSpanId& SpanId)
	{
		const FRPCInfo& RPCInfo = ClassInfoManager->GetRPCInfo(TargetObject, Function);
		const EPushRPCResult Result =
			RPCService->PushRPC(TargetObjectRef.Entity, Sender, RPCInfo.Type, Payload, Channel->bCreatedEntity, TargetObject, Function);

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
			UE_LOG(LogSpatialSender, Log,
				   TEXT("USpatialSender::SendRingBufferedRPC: Ring buffer queue overflowed, dropping RPC. Actor: %s, entity: %lld, "
						"function: %s"),
				   *TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
			return true;
		case EPushRPCResult::HasAckAuthority:
			UE_LOG(
				LogSpatialSender, Warning,
				TEXT("USpatialSender::SendRingBufferedRPC: Worker has authority over ack component for RPC it is sending. RPC will not be "
					 "sent. Actor: %s, entity: %lld, function: %s"),
				*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
			return true;
		case EPushRPCResult::NoRingBufferAuthority:
			// TODO: Change engine logic that calls Client RPCs from non-auth servers and change this to error. UNR-2517
			UE_LOG(
				LogSpatialSender, Log,
				TEXT("USpatialSender::SendRingBufferedRPC: Failed to send RPC because the worker does not have authority over ring buffer "
					 "component. Actor: %s, entity: %lld, function: %s"),
				*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
			return true;
		case EPushRPCResult::EntityBeingCreated:
			UE_LOG(
				LogSpatialSender, Log,
				TEXT(
					"USpatialSender::SendRingBufferedRPC: RPC was called between entity creation and initial authority gain, so it will be "
					"queued. Actor: %s, entity: %lld, function: %s"),
				*TargetObject->GetPathName(), TargetObjectRef.Entity, *Function->GetName());
			return false;
		default:
			return true;
		}
	}

#if !UE_BUILD_SHIPPING
	void USpatialSender::TrackRPC(AActor * Actor, UFunction * Function, const RPCPayload& Payload, const ERPCType RPCType)
	{
		NETWORK_PROFILER(
			GNetworkProfiler.TrackSendRPC(Actor, Function, 0, Payload.CountDataBits(), 0, NetDriver->GetSpatialOSNetConnection()));
		NetDriver->SpatialMetrics->TrackSentRPC(Function, RPCType, Payload.PayloadData.Num());
	}
#endif

	void USpatialSender::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse & Response,
											 const FSpatialGDKSpanId& CauseSpanId)
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
			SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendCommandResponse(RequestId, false),
											 CauseSpanId.GetConstId(), 1);
		}

		Connection->SendCommandFailure(RequestId, Message, SpanId);
	}
