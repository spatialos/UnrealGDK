// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Utils/RPCRingBuffer.h"
#include "Utils/RepLayoutUtils.h"

#include "Interop/RPCs/RPCQueues.h"

DEFINE_LOG_CATEGORY(LogSpatialNetDriverRPC);

using namespace SpatialGDK;

void FRPCMetaData::ComputeSpanId(SpatialGDK::SpatialEventTracer& Tracer, SpatialGDK::EntityComponentId EntityComponent, uint64 RPCId)
{
	TArray<FSpatialGDKSpanId> ComponentUpdateSpans = Tracer.GetAndConsumeSpansForComponent(EntityComponent);

	const Trace_SpanIdType* Causes = reinterpret_cast<const Trace_SpanIdType*>(ComponentUpdateSpans.GetData());
	SpanId = Tracer.TraceEvent(
		RECEIVE_RPC_EVENT_NAME, "", Causes, ComponentUpdateSpans.Num(),
		[this, EntityComponent, RPCId](FSpatialTraceEventDataBuilder& EventBuilder) {
			EventBuilder.AddLinearTraceId(EventTraceUniqueId::GenerateForNamedRPC(EntityComponent.EntityId, RPCName, RPCId));
		});
}

void FRPCPayload::ReadFromSchema(const Schema_Object* RPCObject)
{
	Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
	Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
	PayloadData = GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);
}

void FRPCPayload::WriteToSchema(Schema_Object* RPCObject) const
{
	Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
	Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
	AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, PayloadData.GetData(), PayloadData.Num());
}

void FSpatialNetDriverRPC::OnRPCSent(SpatialGDK::SpatialEventTracer& EventTracer, TArray<UpdateToSend>& OutUpdates, FName Name,
									 Worker_EntityId EntityId, Worker_ComponentId ComponentId, uint64 RPCId,
									 const FSpatialGDKSpanId& SpanId)
{
	FSpatialGDKSpanId NewSpanId =
		EventTracer.TraceEvent(SEND_RPC_EVENT_NAME, "", SpanId.GetConstId(), /* NumCauses */ 1,
							   [EntityId, Name, RPCId](FSpatialTraceEventDataBuilder& EventBuilder) {
								   EventBuilder.AddLinearTraceId(EventTraceUniqueId::GenerateForNamedRPC(EntityId, Name, RPCId));
							   });

	if (OutUpdates.Num() == 0 || OutUpdates.Last().EntityId != EntityId || OutUpdates.Last().Update.component_id != ComponentId)
	{
		UpdateToSend& Update = OutUpdates.AddDefaulted_GetRef();
		Update.EntityId = EntityId;
		Update.Update.component_id = ComponentId;
	}
	OutUpdates.Last().Spans.Add(NewSpanId);
}

void FSpatialNetDriverRPC::OnDataWritten(TArray<FWorkerComponentData>& OutArray, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
										 Schema_ComponentData* InData)
{
	if (ensure(InData != nullptr))
	{
		FWorkerComponentData Data;
		Data.component_id = ComponentId;
		Data.schema_type = InData;
		OutArray.Add(Data);
	}
}

void FSpatialNetDriverRPC::OnUpdateWritten(TArray<UpdateToSend>& OutUpdates, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
										   Schema_ComponentUpdate* InUpdate)
{
	if (ensure(InUpdate != nullptr))
	{
		if (OutUpdates.Num() == 0 || OutUpdates.Last().EntityId != EntityId || OutUpdates.Last().Update.component_id != ComponentId)
		{
			UpdateToSend& Update = OutUpdates.AddDefaulted_GetRef();
			Update.EntityId = EntityId;
			Update.Update.component_id = ComponentId;
		}
		OutUpdates.Last().Update.schema_type = InUpdate;
	}
}

FSpatialNetDriverRPC::StandardQueue::SentRPCCallback FSpatialNetDriverRPC::MakeRPCSentCallback()
{
	check(bUpdateCacheInUse.load());
	if (EventTracer != nullptr)
	{
		return
			[this](FName RPCName, Worker_EntityId EntityId, Worker_ComponentId ComponentId, uint64 RPCId, const FSpatialGDKSpanId& SpanId) {
				return OnRPCSent(*EventTracer, UpdateToSend_Cache, RPCName, EntityId, ComponentId, RPCId, SpanId);
			};
	}
	return StandardQueue::SentRPCCallback();
}

RPCCallbacks::DataWritten FSpatialNetDriverRPC::MakeDataWriteCallback(TArray<FWorkerComponentData>& OutArray) const
{
	return [&OutArray](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* InData) {
		return OnDataWritten(OutArray, EntityId, ComponentId, InData);
	};
}

RPCCallbacks::UpdateWritten FSpatialNetDriverRPC::MakeUpdateWriteCallback()
{
	check(bUpdateCacheInUse.load());
	return [this](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* InUpdate) {
		return OnUpdateWritten(UpdateToSend_Cache, EntityId, ComponentId, InUpdate);
	};
}

FSpatialNetDriverRPC::FSpatialNetDriverRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
										   const SpatialGDK::FSubView& InActorNonAuthSubView)
	: NetDriver(InNetDriver)
{
	EventTracer = NetDriver.Connection->GetEventTracer();
	RPCService = MakeUnique<SpatialGDK::RPCService>(InActorNonAuthSubView, InActorAuthSubView);

	{
		// TODO UNR-5038
		// MulticastReceiver =
	}
}

void FSpatialNetDriverRPC::AdvanceView()
{
	RPCService->AdvanceView();
}

void FSpatialNetDriverRPC::ProcessReceivedRPCs()
{
	// TODO UNR-5038
	// MulticastReceiver->
}

TArray<FWorkerComponentData> FSpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId)
{
	static TArray<Worker_ComponentId> EndpointComponentIds = {
		SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID,
		// TODO UNR-5038, UNR-5040
		/*SpatialConstants::MULTICAST_RPCS_COMPONENT_ID,
		SpatialConstants::CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID*/
	};

	TArray<FWorkerComponentData> Components;
	GetRPCComponentsOnEntityCreation(EntityId, Components);

	for (auto EndpointId : EndpointComponentIds)
	{
		if (!Components.ContainsByPredicate([EndpointId](const FWorkerComponentData& Data) {
				return Data.component_id == EndpointId;
			}))
		{
			FWorkerComponentData Data;
			Data.component_id = EndpointId;
			Data.schema_type = Schema_CreateComponentData();
			Components.Add(Data);
		}
	}

	return Components;
}

void FSpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	// TODO UNR-5038
}

/**
 * Update context object.
 * Responsible for managing the update cache array, and flush updates once the operation is done.
 */
struct FSpatialNetDriverRPC::RAIIUpdateContext : FStackOnly
{
	RAIIUpdateContext(FSpatialNetDriverRPC& RPC)
		: RPCSystem(RPC)
	{
		bool bExpectedValue = false;
		const bool bCacheAvailable = RPCSystem.bUpdateCacheInUse.compare_exchange_strong(bExpectedValue, true);
		check(bCacheAvailable);
	}

	~RAIIUpdateContext()
	{
		const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

		for (auto& Update : RPCSystem.UpdateToSend_Cache)
		{
			FSpatialGDKSpanId SpanId;
			if (RPCSystem.EventTracer != nullptr)
			{
				TArray<FSpatialGDKSpanId>& Causes = Update.Spans;

				SpanId = RPCSystem.EventTracer->TraceEvent(MERGE_SEND_RPCS_EVENT_NAME, "", Causes.GetData()->GetConstId(), Causes.Num(),
														   [Update](FSpatialTraceEventDataBuilder& EventBuilder) {
															   EventBuilder.AddEntityId(Update.EntityId);
															   EventBuilder.AddComponentId(Update.Update.component_id);
														   });
			}
			RPCSystem.NetDriver.Connection->SendComponentUpdate(Update.EntityId, &Update.Update, SpanId);
		}

		if (RPCSystem.UpdateToSend_Cache.Num() > 0 && Settings->bWorkerFlushAfterOutgoingNetworkOp)
		{
			RPCSystem.NetDriver.Connection->Flush();
		}

		// Keep storage around for future frames.
		RPCSystem.UpdateToSend_Cache.SetNum(0, /*bAllowShrinking*/ false);
		RPCSystem.bUpdateCacheInUse.store(false);
	}

	FSpatialNetDriverRPC& RPCSystem;
};

void FSpatialNetDriverRPC::FlushRPCUpdates()
{
	RAIIUpdateContext UpdateCtx(*this);

	const TMap<FName, RPCService::RPCReceiverDescription>& Receivers = RPCService->GetReceivers();
	for (const auto& Receiver : Receivers)
	{
		RPCWritingContext Ctx(Receiver.Key, MakeUpdateWriteCallback());
		Receiver.Value.Receiver->FlushUpdates(Ctx);
	}
}

void FSpatialNetDriverRPC::FlushRPCQueue(StandardQueue& Queue)
{
	RAIIUpdateContext UpdateCtx(*this);

	RPCWritingContext Ctx(Queue.Name, MakeUpdateWriteCallback());
	Queue.FlushAll(Ctx, MakeRPCSentCallback());
}

void FSpatialNetDriverRPC::FlushRPCQueueForEntity(Worker_EntityId EntityId, StandardQueue& Queue)
{
	RAIIUpdateContext UpdateCtx(*this);

	RPCWritingContext Ctx(Queue.Name, MakeUpdateWriteCallback());
	Queue.Flush(EntityId, Ctx, MakeRPCSentCallback());
}

bool FSpatialNetDriverRPC::CanExtractRPC(Worker_EntityId EntityId) const
{
	const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver.PackageMap->GetObjectFromEntityId(EntityId);
	if (!ActorReceivingRPC.IsValid())
	{
		UE_LOG(LogSpatialNetDriverRPC, Log,
			   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
					"destroyed. Entity: %lld"),
			   EntityId);
		return false;
	}
	return true;
}

bool FSpatialNetDriverRPC::CanExtractRPCOnServer(Worker_EntityId EntityId) const
{
	const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver.PackageMap->GetObjectFromEntityId(EntityId);
	UObject* ReceivingObject = ActorReceivingRPC.Get();
	if (ReceivingObject == nullptr)
	{
		UE_LOG(LogSpatialNetDriverRPC, Log,
			   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
					"destroyed. Entity: %lld"),
			   EntityId);
		return false;
	}

	AActor* ReceivingActor = CastChecked<AActor>(ReceivingObject);
	const bool bActorRoleIsSimulatedProxy = ReceivingActor->Role == ROLE_SimulatedProxy;
	if (bActorRoleIsSimulatedProxy)
	{
		UE_LOG(LogSpatialNetDriverRPC, Verbose,
			   TEXT("Will not process server RPC, Actor role changed to SimulatedProxy. This happens on migration. Entity: %lld"),
			   EntityId);
		return false;
	}
	return true;
}

struct RAIIParamsHolder : FStackOnly
{
	RAIIParamsHolder(UFunction& InFunction, uint8* Memory)
		: Function(InFunction)
		, Parms(Memory)
	{
		FMemory::Memzero(Parms, Function.ParmsSize);
	}

	~RAIIParamsHolder()
	{
		// Destroy the parameters.
		// warning: highly dependent on UObject::ProcessEvent freeing of parms!
		for (TFieldIterator<GDK_PROPERTY(Property)> It(&Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Parms);
		}
	}

	UFunction& Function;
	uint8* Parms;
};

bool FSpatialNetDriverRPC::ApplyRPC(Worker_EntityId EntityId, SpatialGDK::ReceivedRPC RPCData, const FRPCMetaData& MetaData) const
{
	constexpr bool RPCConsumed = true;

	FUnrealObjectRef ObjectRef(EntityId, RPCData.Offset);
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver.PackageMap->GetObjectFromUnrealObjectRef(ObjectRef);
	UObject* TargetObject = TargetObjectWeakPtr.Get();

	if (TargetObject == nullptr)
	{
		const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver.PackageMap->GetObjectFromEntityId(EntityId);
		AActor* Actor = CastChecked<AActor>(ActorReceivingRPC.Get());
		checkf(Actor != nullptr, TEXT("Receiving actor should have been checked in CanReceiveRPC"));
		UE_LOG(LogSpatialNetDriverRPC, Error,
			   TEXT("Failed to execute RPC on Actor %s (Entity %llu)'s Subobject %i because the Subobject is null"), *Actor->GetName(),
			   EntityId, RPCData.Offset);

		return RPCConsumed;
	}

	const FClassInfo& ClassInfo = NetDriver.ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[RPCData.Index];
	if (Function == nullptr)
	{
		UE_LOG(LogSpatialNetDriverRPC, Error, TEXT("Failed to execute RPC on Actor %s, (Entity %llu), function missing for index %i"),
			   *TargetObject->GetName(), EntityId, RPCData.Index);
		return RPCConsumed;
	}

	// NB : FMemory_Alloca is stack allocation, and cannot be done inside the RAII holder.
	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	RAIIParamsHolder ParamAlloc(*Function, Parms);

	TSet<FUnrealObjectRef> UnresolvedRefs;
	{
		// Scope for the SpatialNetBitReader lifecycle (only one should exist per thread).
		TSet<FUnrealObjectRef> MappedRefs;
		constexpr uint32 BitsPerByte = 8;

		FSpatialNetBitReader PayloadReader(NetDriver.PackageMap, const_cast<uint8*>(RPCData.PayloadData.GetData()),
										   RPCData.PayloadData.Num() * BitsPerByte, MappedRefs, UnresolvedRefs);

		TSharedPtr<FRepLayout> RepLayout = NetDriver.GetFunctionRepLayout(Function);
		check(RepLayout.IsValid());
		RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);
	}

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	const float TimeQueued = (FPlatformTime::Cycles64() - MetaData.Timestamp) * FPlatformTime::GetSecondsPerCycle64();
	const int32 UnresolvedRefCount = UnresolvedRefs.Num();

	if (UnresolvedRefCount != 0 && TimeQueued < SpatialSettings->QueuedIncomingRPCWaitTime)
	{
		return !RPCConsumed;
	}

	TOptional<ERPCType> RPCType = SpatialConstants::RPCStringToType(MetaData.RPCName.ToString());

	if (UnresolvedRefCount > 0 && RPCType.IsSet() && !SpatialSettings->ShouldRPCTypeAllowUnresolvedParameters(RPCType.GetValue())
		&& (Function->SpatialFunctionFlags & SPATIALFUNC_AllowUnresolvedParameters) == 0)
	{
		const FString UnresolvedEntityIds = FString::JoinBy(UnresolvedRefs, TEXT(", "), [](const FUnrealObjectRef& Ref) {
			return Ref.ToString();
		});

		UE_LOG(LogSpatialNetDriverRPC, Warning,
			   TEXT("Executed RPC %s::%s with unresolved references (%s) after %.3f seconds of queueing. Owner name: %s"),
			   *GetNameSafe(TargetObject), *GetNameSafe(Function), *UnresolvedEntityIds, TimeQueued,
			   *GetNameSafe(TargetObject->GetOuter()));
	}

	const bool bUseEventTracer = EventTracer != nullptr;
	if (bUseEventTracer)
	{
		FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(APPLY_RPC_EVENT_NAME, "", MetaData.SpanId.GetConstId(), /* NumCauses */ 1,
														   [TargetObject, Function](FSpatialTraceEventDataBuilder& EventBuilder) {
															   EventBuilder.AddObject(TargetObject);
															   EventBuilder.AddFunction(Function);
														   });
		EventTracer->AddToStack(SpanId);
	}

	TargetObject->ProcessEvent(Function, Parms);

	if (bUseEventTracer)
	{
		EventTracer->PopFromStack();
	}

	return RPCConsumed;
}

void FSpatialNetDriverRPC::MakeRingBufferWithACKSender(ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
													   TUniquePtr<RPCBufferSender>& SenderPtr,
													   TUniquePtr<TRPCQueue<FRPCPayload, FSpatialGDKSpanId>>& QueuePtr)
{
	// TODO UNR-5037
}

void FSpatialNetDriverRPC::MakeRingBufferWithACKReceiver(ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
														 TUniquePtr<TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>>& ReceiverPtr)
{
	// TODO UNR-5037
}

FSpatialNetDriverServerRPC::FSpatialNetDriverServerRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
													   const SpatialGDK::FSubView& InActorNonAuthSubView)
	: FSpatialNetDriverRPC(InNetDriver, InActorAuthSubView, InActorNonAuthSubView)
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	auto constexpr RequiredAuth = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;

	MakeRingBufferWithACKSender(ERPCType::ClientReliable, RequiredAuth, ClientReliableSender, ClientReliableQueue);
	MakeRingBufferWithACKSender(ERPCType::ClientUnreliable, RequiredAuth, ClientUnreliableSender, ClientUnreliableQueue);

	MakeRingBufferWithACKReceiver(ERPCType::ServerReliable, RequiredAuth, ServerReliableReceiver);
	MakeRingBufferWithACKReceiver(ERPCType::ServerUnreliable, RequiredAuth, ServerUnreliableReceiver);

	{
		auto RPCType = ERPCType::NetMulticast;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		// TODO UNR-5038
	}
}

void FSpatialNetDriverServerRPC::FlushRPCUpdates()
{
	FSpatialNetDriverRPC::FlushRPCUpdates();
	FlushRPCQueue(*ClientReliableQueue);
	FlushRPCQueue(*ClientUnreliableQueue);
}

void FSpatialNetDriverServerRPC::ProcessReceivedRPCs()
{
	FSpatialNetDriverRPC::ProcessReceivedRPCs();

	auto CanExtractRPCs = [this](Worker_EntityId EntityId) {
		return CanExtractRPCOnServer(EntityId);
	};
	auto ProcessRPC = [this](Worker_EntityId EntityId, ReceivedRPC RPCData, const FRPCMetaData& MetaData) {
		return ApplyRPC(EntityId, RPCData, MetaData);
	};

	ServerReliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
	ServerUnreliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
}

void FSpatialNetDriverServerRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	FSpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(EntityId, OutData);

	{
		RPCWritingContext Ctx(ClientReliableQueue->Name, MakeDataWriteCallback(OutData));
		ClientReliableQueue->Flush(EntityId, Ctx, StandardQueue::SentRPCCallback(), /*bIgnoreAdded*/ true);
	}
	{
		RPCWritingContext Ctx(ClientUnreliableQueue->Name, MakeDataWriteCallback(OutData));
		ClientUnreliableQueue->Flush(EntityId, Ctx, StandardQueue::SentRPCCallback(), /*bIgnoreAdded*/ true);
	}
}

FSpatialNetDriverClientRPC::FSpatialNetDriverClientRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
													   const SpatialGDK::FSubView& InActorNonAuthSubView)
	: FSpatialNetDriverRPC(InNetDriver, InActorAuthSubView, InActorNonAuthSubView)
{
	auto constexpr RequiredAuth = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;

	MakeRingBufferWithACKSender(ERPCType::ServerReliable, RequiredAuth, ServerReliableSender, ServerReliableQueue);
	MakeRingBufferWithACKSender(ERPCType::ServerUnreliable, RequiredAuth, ServerUnreliableSender, ServerUnreliableQueue);

	MakeRingBufferWithACKReceiver(ERPCType::ClientReliable, RequiredAuth, ClientReliableReceiver);
	MakeRingBufferWithACKReceiver(ERPCType::ClientUnreliable, RequiredAuth, ClientUnreliableReceiver);
}

void FSpatialNetDriverClientRPC::FlushRPCUpdates()
{
	FSpatialNetDriverRPC::FlushRPCUpdates();
	FlushRPCQueue(*ServerReliableQueue);
	FlushRPCQueue(*ServerUnreliableQueue);
}

void FSpatialNetDriverClientRPC::ProcessReceivedRPCs()
{
	FSpatialNetDriverRPC::ProcessReceivedRPCs();

	auto CanExtractRPCs = [this](Worker_EntityId EntityId) {
		return CanExtractRPC(EntityId);
	};
	auto ProcessRPC = [this](Worker_EntityId EntityId, ReceivedRPC RPCData, const FRPCMetaData& MetaData) {
		return ApplyRPC(EntityId, RPCData, MetaData);
	};

	ClientReliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
	ClientUnreliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
}

void FSpatialNetDriverClientRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	// Clients should not create entities
	checkNoEntry();
}
