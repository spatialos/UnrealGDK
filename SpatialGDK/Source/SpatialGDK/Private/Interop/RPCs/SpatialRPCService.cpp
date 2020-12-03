// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCs/SpatialRPCService.h"

#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/SpatialStaticComponentView.h"
#include "SpatialConstants.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialLatencyTracer.h"

DEFINE_LOG_CATEGORY(LogSpatialRPCService);

namespace SpatialGDK
{
SpatialRPCService::SpatialRPCService(const FSubView& InActorAuthSubView, const FSubView& InActorNonAuthSubView,
									 USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer,
									 USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
	, SpatialLatencyTracer(InSpatialLatencyTracer)
	, EventTracer(InEventTracer)
	, RPCStore(FRPCStore())
	, ClientServerRPCs(ExtractRPCDelegate::CreateRaw(this, &SpatialRPCService::ProcessOrQueueIncomingRPC), InActorAuthSubView, InNetDriver,
					   RPCStore)
	, MulticastRPCs(ExtractRPCDelegate::CreateRaw(this, &SpatialRPCService::ProcessOrQueueIncomingRPC), InActorNonAuthSubView, RPCStore)
	, AuthSubView(&InActorAuthSubView)
	, LastProcessingTime(-GetDefault<USpatialGDKSettings>()->QueuedIncomingRPCRetryTime)
{
	IncomingRPCs.BindProcessingFunction(FProcessRPCDelegate::CreateRaw(this, &SpatialRPCService::ApplyRPC));
}

void SpatialRPCService::AdvanceView()
{
	ClientServerRPCs.AdvanceView();
	MulticastRPCs.AdvanceView();
}

void SpatialRPCService::ProcessChanges(const float NetDriverTime)
{
	ClientServerRPCs.ProcessChanges();
	MulticastRPCs.ProcessChanges();

	if (NetDriverTime - LastProcessingTime > GetDefault<USpatialGDKSettings>()->QueuedIncomingRPCRetryTime)
	{
		LastProcessingTime = NetDriverTime;
		ProcessIncomingRPCs();
	}
}

void SpatialRPCService::ProcessIncomingRPCs()
{
	IncomingRPCs.ProcessRPCs();
}

EPushRPCResult SpatialRPCService::PushRPC(const Worker_EntityId EntityId, const ERPCType Type, RPCPayload Payload,
										  const bool bCreatedEntity, UObject* Target, UFunction* Function)
{
	const EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	EPushRPCResult Result = EPushRPCResult::Success;
	PendingRPCPayload PendingPayload = { Payload };

	if (EventTracer != nullptr)
	{
		PendingPayload.SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreatePushRPC(Target, Function),
														EventTracer->GetFromStack().GetConstId(), 1);
	}

#if TRACE_LIB_ACTIVE
	TraceKey Trace = Payload.Trace;
#endif

	if (RPCRingBufferUtils::ShouldQueueOverflowed(Type) && ClientServerRPCs.ContainsOverflowedRPC(EntityType))
	{
		if (EventTracer != nullptr)
		{
			PendingPayload.SpanId =
				EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateQueueRPC(), PendingPayload.SpanId.GetConstId(), 1);
		}

		// Already has queued RPCs of this type, queue until those are pushed.
		ClientServerRPCs.AddOverflowedRPC(EntityType, MoveTemp(PendingPayload));
		Result = EPushRPCResult::QueueOverflowed;
	}
	else
	{
		Result = PushRPCInternal(EntityId, Type, PendingPayload, bCreatedEntity);
		if (Result == EPushRPCResult::QueueOverflowed)
		{
			ClientServerRPCs.AddOverflowedRPC(EntityType, MoveTemp(PendingPayload));
		}
	}

#if TRACE_LIB_ACTIVE
	ProcessResultToLatencyTrace(Result, Trace);
#endif

	return Result;
}

void SpatialRPCService::PushOverflowedRPCs()
{
	for (auto It = ClientServerRPCs.GetOverflowedRPCs().CreateIterator(); It; ++It)
	{
		Worker_EntityId EntityId = It.Key().EntityId;
		ERPCType Type = It.Key().Type;
		TArray<PendingRPCPayload>& OverflowedRPCArray = It.Value();

		int NumProcessed = 0;
		bool bShouldDrop = false;
		for (PendingRPCPayload& Payload : OverflowedRPCArray)
		{
			const EPushRPCResult Result = PushRPCInternal(EntityId, Type, Payload, false);

			switch (Result)
			{
			case EPushRPCResult::Success:
				NumProcessed++;
				break;
			case EPushRPCResult::QueueOverflowed:
				if (NumProcessed > 0)
				{
					UE_LOG(LogSpatialRPCService, Log,
						   TEXT("SpatialRPCService::PushOverflowedRPCs: Sent some but not all overflowed RPCs. RPCs sent %d, RPCs still "
								"overflowed: %d, Entity: %lld, RPC type: %s"),
						   NumProcessed, OverflowedRPCArray.Num() - NumProcessed, EntityId, *SpatialConstants::RPCTypeToString(Type));
				}
				if (EventTracer != nullptr)
				{
					Payload.SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateQueueRPC(), Payload.SpanId.GetConstId(), 1);
				}
				break;
			case EPushRPCResult::DropOverflowed:
				checkf(false, TEXT("Shouldn't be able to drop on overflow for RPC type that was previously queued."));
				break;
			case EPushRPCResult::HasAckAuthority:
				UE_LOG(LogSpatialRPCService, Warning,
					   TEXT("SpatialRPCService::PushOverflowedRPCs: Gained authority over ack component for RPC type that was overflowed. "
							"Entity: %lld, RPC type: %s"),
					   EntityId, *SpatialConstants::RPCTypeToString(Type));
				bShouldDrop = true;
				break;
			case EPushRPCResult::NoRingBufferAuthority:
				UE_LOG(LogSpatialRPCService, Warning,
					   TEXT("SpatialRPCService::PushOverflowedRPCs: Lost authority over ring buffer component for RPC type that was "
							"overflowed. Entity: %lld, RPC type: %s"),
					   EntityId, *SpatialConstants::RPCTypeToString(Type));
				bShouldDrop = true;
				break;
			default:
				checkNoEntry();
			}
#if TRACE_LIB_ACTIVE
			ProcessResultToLatencyTrace(Result, Payload.Payload.Trace);
#endif

			// This includes the valid case of RPCs still overflowing (EPushRPCResult::QueueOverflowed), as well as the error cases.
			if (Result != EPushRPCResult::Success)
			{
				break;
			}
		}

		if (NumProcessed == OverflowedRPCArray.Num() || bShouldDrop)
		{
			It.RemoveCurrent();
		}
		else
		{
			OverflowedRPCArray.RemoveAt(0, NumProcessed);
		}
	}
}

TArray<SpatialRPCService::UpdateToSend> SpatialRPCService::GetRPCsAndAcksToSend()
{
	TArray<UpdateToSend> UpdatesToSend;

	for (auto& It : RPCStore.PendingComponentUpdatesToSend)
	{
		UpdateToSend& UpdateToSend = UpdatesToSend.AddZeroed_GetRef();
		UpdateToSend.EntityId = It.Key.EntityId;
		UpdateToSend.Update.component_id = It.Key.ComponentId;
		UpdateToSend.Update.schema_type = It.Value.Update;

		if (EventTracer != nullptr)
		{
			UpdateToSend.SpanId = EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateMergeSendRPCs(UpdateToSend.EntityId, UpdateToSend.Update.component_id),
				It.Value.SpanIds.GetData()->GetConstId(), It.Value.SpanIds.Num());
		}

#if TRACE_LIB_ACTIVE
		TraceKey Trace = InvalidTraceKey;
		PendingTraces.RemoveAndCopyValue(It.Key, Trace);
		UpdateToSend.Update.Trace = Trace;
#endif
	}

	RPCStore.PendingComponentUpdatesToSend.Empty();

	return UpdatesToSend;
}

TArray<FWorkerComponentData> SpatialRPCService::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId)
{
	static TArray<Worker_ComponentId> EndpointComponentIds = { SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID,
															   SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
															   SpatialConstants::MULTICAST_RPCS_COMPONENT_ID };

	TArray<FWorkerComponentData> Components;

	for (Worker_ComponentId EndpointComponentId : EndpointComponentIds)
	{
		const EntityComponentId EntityComponent = { EntityId, EndpointComponentId };

		FWorkerComponentData& Component = Components.Emplace_GetRef(FWorkerComponentData{});
		Component.component_id = EndpointComponentId;
		if (Schema_ComponentData** ComponentData = RPCStore.PendingRPCsOnEntityCreation.Find(EntityComponent))
		{
			// When sending initial multicast RPCs, write the number of RPCs into a separate field instead of
			// last sent RPC ID field. When the server gains authority for the first time, it will copy the
			// value over to last sent RPC ID, so the clients that checked out the entity process the initial RPCs.
			if (EndpointComponentId == SpatialConstants::MULTICAST_RPCS_COMPONENT_ID)
			{
				RPCRingBufferUtils::MoveLastSentIdToInitiallyPresentCount(
					Schema_GetComponentDataFields(*ComponentData),
					RPCStore.LastSentRPCIds[EntityRPCType(EntityId, ERPCType::NetMulticast)]);
			}

			if (EndpointComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID)
			{
				UE_LOG(LogSpatialRPCService, Error,
					   TEXT("SpatialRPCService::GetRPCComponentsOnEntityCreation: Initial RPCs present on ClientEndpoint! EntityId: %lld"),
					   EntityId);
			}

			Component.schema_type = *ComponentData;
#if TRACE_LIB_ACTIVE
			TraceKey Trace = InvalidTraceKey;
			PendingTraces.RemoveAndCopyValue(EntityComponent, Trace);
			Component.Trace = Trace;
#endif
			RPCStore.PendingRPCsOnEntityCreation.Remove(EntityComponent);
		}
		else
		{
			Component.schema_type = Schema_CreateComponentData();
		}
	}

	return Components;
}

void SpatialRPCService::ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload,
												  TOptional<uint64> RPCIdForLinearEventTrace)
{
	const TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(InTargetObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		UE_LOG(LogSpatialRPCService, Verbose, TEXT("The object has been deleted, dropping the RPC"));
		return;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);

	if (InPayload.Index >= static_cast<uint32>(ClassInfo.RPCs.Num()))
	{
		// This should only happen if there's a class layout disagreement between workers, which would indicate incompatible binaries.
		UE_LOG(LogSpatialRPCService, Error, TEXT("Invalid RPC index (%d) received on %s, dropping the RPC"), InPayload.Index,
			   *TargetObject->GetPathName());
		return;
	}
	UFunction* Function = ClassInfo.RPCs[InPayload.Index];
	if (Function == nullptr)
	{
		UE_LOG(LogSpatialRPCService, Error, TEXT("Missing function info received on %s, dropping the RPC"), *TargetObject->GetPathName());
		return;
	}

	const FRPCInfo& RPCInfo = NetDriver->ClassInfoManager->GetRPCInfo(TargetObject, Function);
	const ERPCType Type = RPCInfo.Type;

	IncomingRPCs.ProcessOrQueueRPC(InTargetObjectRef, Type, MoveTemp(InPayload), RPCIdForLinearEventTrace);
}

void SpatialRPCService::ClearPendingRPCs(Worker_EntityId EntityId)
{
	IncomingRPCs.DropForEntity(EntityId);
}

EPushRPCResult SpatialRPCService::PushRPCInternal(const Worker_EntityId EntityId, const ERPCType Type, PendingRPCPayload Payload,
												  const bool bCreatedEntity)
{
	const Worker_ComponentId RingBufferComponentId = RPCRingBufferUtils::GetRingBufferComponentId(Type);

	const EntityComponentId EntityComponent = { EntityId, RingBufferComponentId };
	const EntityRPCType EntityType = EntityRPCType(EntityId, Type);

	Schema_Object* EndpointObject;
	uint64 LastAckedRPCId;
	if (AuthSubView->HasComponent(EntityId, RingBufferComponentId))
	{
		if (!AuthSubView->HasAuthority(EntityId, RingBufferComponentId))
		{
			if (bCreatedEntity)
			{
				return EPushRPCResult::EntityBeingCreated;
			}
			return EPushRPCResult::NoRingBufferAuthority;
		}

		EndpointObject = Schema_GetComponentUpdateFields(RPCStore.GetOrCreateComponentUpdate(EntityComponent, Payload.SpanId));

		if (Type == ERPCType::NetMulticast)
		{
			// Assume all multicast RPCs are auto-acked.
			LastAckedRPCId = RPCStore.LastSentRPCIds.FindRef(EntityType);
		}
		else
		{
			// We shouldn't have authority over the component that has the acks.
			if (AuthSubView->HasAuthority(EntityId, RPCRingBufferUtils::GetAckComponentId(Type)))
			{
				return EPushRPCResult::HasAckAuthority;
			}

			LastAckedRPCId = ClientServerRPCs.GetAckFromView(EntityId, Type);
		}
	}
	else
	{
		if (bCreatedEntity)
		{
			return EPushRPCResult::EntityBeingCreated;
		}
		// If the entity isn't in the view, we assume this RPC was called before
		// CreateEntityRequest, so we put it into a component data object.
		EndpointObject = Schema_GetComponentDataFields(RPCStore.GetOrCreateComponentData(EntityComponent));

		LastAckedRPCId = 0;
	}

	const uint64 NewRPCId = RPCStore.LastSentRPCIds.FindRef(EntityType) + 1;

	// Check capacity.
	if (LastAckedRPCId + RPCRingBufferUtils::GetRingBufferSize(Type) >= NewRPCId)
	{
		if (EventTracer != nullptr)
		{
			EventTraceUniqueId LinearTraceId = EventTraceUniqueId::GenerateForRPC(EntityId, static_cast<uint8>(Type), NewRPCId);
			FSpatialGDKSpanId SpanId =
				EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateSendRPC(LinearTraceId), Payload.SpanId.GetConstId(), 1);
			RPCStore.AddSpanIdForComponentUpdate(EntityComponent, SpanId);
		}

		RPCRingBufferUtils::WriteRPCToSchema(EndpointObject, Type, NewRPCId, Payload.Payload);

#if TRACE_LIB_ACTIVE
		if (SpatialLatencyTracer != nullptr && Payload.Payload.Trace != InvalidTraceKey)
		{
			if (PendingTraces.Find(EntityComponent) == nullptr)
			{
				PendingTraces.Add(EntityComponent, Payload.Payload.Trace);
			}
			else
			{
				SpatialLatencyTracer->WriteAndEndTrace(Payload.Payload.Trace,
													   TEXT("Multiple rpc updates in single update, ending further stack tracing"), true);
			}
		}
#endif

		RPCStore.LastSentRPCIds.Add(EntityType, NewRPCId);
	}
	else
	{
		// Overflowed
		if (RPCRingBufferUtils::ShouldQueueOverflowed(Type))
		{
			return EPushRPCResult::QueueOverflowed;
		}
		else
		{
			return EPushRPCResult::DropOverflowed;
		}
	}

	return EPushRPCResult::Success;
}

FRPCErrorInfo SpatialRPCService::ApplyRPC(const FPendingRPCParams& Params)
{
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return FRPCErrorInfo{ nullptr, nullptr, ERPCResult::UnresolvedTargetObject, ERPCQueueProcessResult::StopProcessing };
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return FRPCErrorInfo{ TargetObject, nullptr, ERPCResult::MissingFunctionInfo, ERPCQueueProcessResult::ContinueProcessing };
	}

	return ApplyRPCInternal(TargetObject, Function, Params);
}

FRPCErrorInfo SpatialRPCService::ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams)
{
	FRPCErrorInfo ErrorInfo = { TargetObject, Function, ERPCResult::UnresolvedParameters };

	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	FMemory::Memzero(Parms, Function->ParmsSize);

	TSet<FUnrealObjectRef> UnresolvedRefs;
	{
		TSet<FUnrealObjectRef> MappedRefs;
		RPCPayload PayloadCopy = PendingRPCParams.Payload;
		FSpatialNetBitReader PayloadReader(NetDriver->PackageMap, PayloadCopy.PayloadData.GetData(), PayloadCopy.CountDataBits(),
										   MappedRefs, UnresolvedRefs);

		TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
		RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);
	}

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	const float TimeQueued = (FDateTime::Now() - PendingRPCParams.Timestamp).GetTotalSeconds();
	const int32 UnresolvedRefCount = UnresolvedRefs.Num();

	if (UnresolvedRefCount == 0 || SpatialSettings->QueuedIncomingRPCWaitTime < TimeQueued)
	{
		if (UnresolvedRefCount > 0 && !SpatialSettings->ShouldRPCTypeAllowUnresolvedParameters(PendingRPCParams.Type)
			&& (Function->SpatialFunctionFlags & SPATIALFUNC_AllowUnresolvedParameters) == 0)
		{
			const FString UnresolvedEntityIds = FString::JoinBy(UnresolvedRefs, TEXT(", "), [](const FUnrealObjectRef& Ref) {
				return Ref.ToString();
			});

			UE_LOG(LogSpatialRPCService, Warning,
				   TEXT("Executed RPC %s::%s with unresolved references (%s) after %.3f seconds of queueing. Owner name: %s"),
				   *GetNameSafe(TargetObject), *GetNameSafe(Function), *UnresolvedEntityIds, TimeQueued,
				   *GetNameSafe(TargetObject->GetOuter()));
		}

		// Get the RPC target Actor.
		AActor* Actor = TargetObject->IsA<AActor>() ? Cast<AActor>(TargetObject) : TargetObject->GetTypedOuter<AActor>();
		ERPCType RPCType = PendingRPCParams.Type;

		if (Actor->Role == ROLE_SimulatedProxy && (RPCType == ERPCType::ServerReliable || RPCType == ERPCType::ServerUnreliable))
		{
			ErrorInfo.ErrorCode = ERPCResult::NoAuthority;
			ErrorInfo.QueueProcessResult = ERPCQueueProcessResult::DropEntireQueue;
		}
		else
		{
			bool bUseEventTracer =
				EventTracer != nullptr && RPCType != ERPCType::CrossServer && PendingRPCParams.RPCIdForLinearEventTrace.IsSet();
			if (bUseEventTracer)
			{
				Worker_ComponentId ComponentId = RPCRingBufferUtils::GetRingBufferComponentId(RPCType);
				EntityComponentId Id = EntityComponentId(PendingRPCParams.ObjectRef.Entity, ComponentId);
				FSpatialGDKSpanId CauseSpanId = EventTracer->GetSpanId(Id);

				EventTraceUniqueId LinearTraceId = EventTraceUniqueId::GenerateForRPC(
					PendingRPCParams.ObjectRef.Entity, static_cast<uint8>(RPCType), PendingRPCParams.RPCIdForLinearEventTrace.GetValue());
				FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(
					FSpatialTraceEventBuilder::CreateProcessRPC(TargetObject, Function, LinearTraceId), CauseSpanId.GetConstId(), 1);
				EventTracer->AddToStack(SpanId);
			}

			TargetObject->ProcessEvent(Function, Parms);

			if (bUseEventTracer)
			{
				EventTracer->PopFromStack();
			}

			if (RPCType != ERPCType::CrossServer && RPCType != ERPCType::NetMulticast)
			{
				ClientServerRPCs.IncrementAckedRPCID(PendingRPCParams.ObjectRef.Entity, RPCType);
			}

			ErrorInfo.ErrorCode = ERPCResult::Success;
		}
	}

	// Destroy the parameters.
	// warning: highly dependent on UObject::ProcessEvent freeing of parms!
	for (TFieldIterator<GDK_PROPERTY(Property)> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}

	return ErrorInfo;
}

#if TRACE_LIB_ACTIVE
void SpatialRPCService::ProcessResultToLatencyTrace(const EPushRPCResult Result, const TraceKey Trace)
{
	if (SpatialLatencyTracer != nullptr && Trace != InvalidTraceKey)
	{
		bool bEndTrace = false;
		FString TraceMsg;
		switch (Result)
		{
		case SpatialGDK::EPushRPCResult::Success:
			// No further action
			break;
		case SpatialGDK::EPushRPCResult::QueueOverflowed:
			TraceMsg = TEXT("Overflowed");
			break;
		case SpatialGDK::EPushRPCResult::DropOverflowed:
			TraceMsg = TEXT("OverflowedAndDropped");
			bEndTrace = true;
			break;
		case SpatialGDK::EPushRPCResult::HasAckAuthority:
			TraceMsg = TEXT("NoAckAuth");
			bEndTrace = true;
			break;
		case SpatialGDK::EPushRPCResult::NoRingBufferAuthority:
			TraceMsg = TEXT("NoRingBufferAuth");
			bEndTrace = true;
			break;
		default:
			TraceMsg = TEXT("UnrecognisedResult");
			break;
		}

		if (bEndTrace)
		{
			// This RPC has been dropped, end the trace
			SpatialLatencyTracer->WriteAndEndTrace(Trace, TraceMsg, false);
		}
		else if (!TraceMsg.IsEmpty())
		{
			// This RPC will be sent later
			SpatialLatencyTracer->WriteToLatencyTrace(Trace, TraceMsg);
		}
	}
}
#endif // TRACE_LIB_ACTIVE
} // namespace SpatialGDK
