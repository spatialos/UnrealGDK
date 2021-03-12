#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/RPCRingBuffer.h"

#include "Interop/RPCs/RPCQueues.h"

using namespace SpatialGDK;

void FRPCMetaData::ComputeSpanId(FName Name, SpatialGDK::SpatialEventTracer& Tracer, SpatialGDK::EntityComponentId EntityComponent,
								 uint64 RPCId)
{
	TArray<FSpatialGDKSpanId> ComponentUpdateSpans = Tracer.GetAndConsumeSpansForComponent(EntityComponent);

	SpanId = Tracer.TraceEvent(
		FSpatialTraceEventBuilder::CreateReceiveRPC(EventTraceUniqueId::GenerateForNamedRPC(EntityComponent.EntityId, Name, RPCId)),
		/* Causes */ reinterpret_cast<const Trace_SpanIdType*>(ComponentUpdateSpans.GetData()),
		/* NumCauses */ ComponentUpdateSpans.Num());
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

USpatialNetDriverRPC::USpatialNetDriverRPC() = default;

void USpatialNetDriverRPC::OnRPCSent(SpatialGDK::SpatialEventTracer& EventTracer, TArray<UpdateToSend>& OutUpdates, FName Name,
									 Worker_EntityId EntityId, Worker_ComponentId ComponentId, uint64 RPCId,
									 const FSpatialGDKSpanId& SpanId)
{
	const EventTraceUniqueId LinearTraceId = EventTraceUniqueId::GenerateForNamedRPC(EntityId, Name, RPCId);
	const FSpatialGDKSpanId NewSpanId = EventTracer.TraceEvent(FSpatialTraceEventBuilder::CreateSendRPC(LinearTraceId),
														  /* Causes */ SpanId.GetConstId(), /* NumCauses */ 1);

	if (OutUpdates.Num() == 0 || OutUpdates.Last().EntityId != EntityId || OutUpdates.Last().Update.component_id != ComponentId)
	{
		UpdateToSend& Update = OutUpdates.AddDefaulted_GetRef();
		Update.EntityId = EntityId;
		Update.Update.component_id = ComponentId;
	}
	OutUpdates.Last().Spans.Add(NewSpanId);
}

void USpatialNetDriverRPC::OnDataWritten(TArray<FWorkerComponentData>& OutArray, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
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

void USpatialNetDriverRPC::OnUpdateWritten(TArray<UpdateToSend>& OutUpdates, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
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

USpatialNetDriverRPC::StandardQueue::SentRPCCallback USpatialNetDriverRPC::MakeRPCSentCallback(TArray<UpdateToSend>& OutUpdates)
{
	if (EventTracer != nullptr)
	{
		return [&OutUpdates, this](FName Name, Worker_EntityId EntityId, Worker_ComponentId ComponentId, uint64 RPCId,
								   const FSpatialGDKSpanId& SpanId) {
			return OnRPCSent(*EventTracer, OutUpdates, Name, EntityId, ComponentId, RPCId, SpanId);
		};
	}
	return StandardQueue::SentRPCCallback();
}

RPCCallbacks::DataWritten USpatialNetDriverRPC::MakeDataWriteCallback(TArray<FWorkerComponentData>& OutArray) const
{
	return [&OutArray](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentData* InData) {
		return OnDataWritten(OutArray, EntityId, ComponentId, InData);
	};
}

RPCCallbacks::UpdateWritten USpatialNetDriverRPC::MakeUpdateWriteCallback(TArray<UpdateToSend>& OutUpdates) const
{
	return [&OutUpdates](Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_ComponentUpdate* InUpdate) {
		return OnUpdateWritten(OutUpdates, EntityId, ComponentId, InUpdate);
	};
}

void USpatialNetDriverRPC::Init(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
								const SpatialGDK::FSubView& InActorNonAuthSubView)
{
	NetDriver = &InNetDriver;
	EventTracer = NetDriver->Connection->GetEventTracer();
	RPCService = MakeUnique<SpatialGDK::RPCService>(InActorNonAuthSubView, InActorAuthSubView);

	{
		// TODO UNR-5038
		// MulticastReceiver =
	}
}

void USpatialNetDriverRPC::AdvanceView()
{
	RPCService->AdvanceView();
}

void USpatialNetDriverRPC::ProcessReceivedRPCs()
{
	// TODO UNR-5038
	// MulticastReceiver->
}

TArray<FWorkerComponentData> USpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId)
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

void USpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	// TODO UNR-5038
}

void USpatialNetDriverRPC::FlushRPCUpdates()
{
	const TMap<FName, RPCService::RPCReceiverDescription>& Receivers = RPCService->GetReceivers();
	TArray<UpdateToSend> Updates;
	for (const auto& Receiver : Receivers)
	{
		RPCWritingContext Ctx(Receiver.Key, MakeUpdateWriteCallback(Updates));
		Receiver.Value.Receiver->FlushUpdates(Ctx);
	}

	FlushUpdates(Updates);
}

void USpatialNetDriverRPC::FlushRPCQueue(StandardQueue& Queue)
{
	TArray<UpdateToSend> Updates;
	RPCWritingContext Ctx(Queue.Name, MakeUpdateWriteCallback(Updates));
	Queue.FlushAll(Ctx, MakeRPCSentCallback(Updates));

	FlushUpdates(Updates);
}

void USpatialNetDriverRPC::FlushRPCQueue(Worker_EntityId EntityId, StandardQueue& Queue)
{
	TArray<UpdateToSend> Updates;
	RPCWritingContext Ctx(Queue.Name, MakeUpdateWriteCallback(Updates));
	Queue.Flush(EntityId, Ctx, MakeRPCSentCallback(Updates));

	FlushUpdates(Updates);
}

void USpatialNetDriverRPC::FlushUpdates(TArray<UpdateToSend>& Updates)
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	for (auto& Update : Updates)
	{
		FSpatialGDKSpanId SpanId;
		if (EventTracer != nullptr)
		{
			SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateMergeSendRPCs(Update.EntityId, Update.Update.component_id),
											 /* Causes */ Update.Spans.GetData()->GetConstId(), /* NumCauses */ Update.Spans.Num());
		}
		NetDriver->Connection->SendComponentUpdate(Update.EntityId, &Update.Update, SpanId);
	}

	if (Updates.Num() > 0 && Settings->bWorkerFlushAfterOutgoingNetworkOp)
	{
		NetDriver->Connection->Flush();
	}
}

bool USpatialNetDriverRPC::CanExtractRPC(Worker_EntityId EntityId)
{
	const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver->PackageMap->GetObjectFromEntityId(EntityId);
	if (!ActorReceivingRPC.IsValid())
	{
		//UE_LOG(LogSpatialRPCService, Log,
		//	   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
		//			"destroyed. Entity: %lld"),
		//	   EntityId);
		return false;
	}
	return true;
}

bool USpatialNetDriverRPC::CanExtractRPCOnServer(Worker_EntityId EntityId)
{
	const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver->PackageMap->GetObjectFromEntityId(EntityId);
	UObject* ReceivingObject = ActorReceivingRPC.Get();
	if (ReceivingObject == nullptr)
	{
		//UE_LOG(LogSpatialRPCService, Log,
		//	   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
		//			"destroyed. Entity: %lld"),
		//	   EntityId);
		return false;
	}

	AActor* ReceivingActor = CastChecked<AActor>(ReceivingObject);
	const bool bActorRoleIsSimulatedProxy = ReceivingActor->Role == ROLE_SimulatedProxy;
	if (bActorRoleIsSimulatedProxy)
	{
		//UE_LOG(LogSpatialRPCService, Verbose,
		//	   TEXT("Will not process server RPC, Actor role changed to SimulatedProxy. This happens on migration. Entity: %lld"),
		//	   EntityId);
		return false;
	}
	return true;
}

struct RAIIParamsHolder
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

bool USpatialNetDriverRPC::ApplyRPC(Worker_EntityId EntityId, SpatialGDK::ReceivedRPC RPCData, const FRPCMetaData& MetaData)
{
	constexpr bool RPCConsumed = true;

	FUnrealObjectRef ObjectRef(EntityId, RPCData.Offset);
	TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(ObjectRef);
	UObject* TargetObject = TargetObjectWeakPtr.Get();

	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[RPCData.Index];
	if (Function == nullptr)
	{
		// TODO, log something
		// return FRPCErrorInfo{ TargetObject, nullptr, ERPCResult::MissingFunctionInfo, ERPCQueueProcessResult::ContinueProcessing };
		return RPCConsumed;
	}

	// NB : FMemory_Alloca is stack allocation, and cannot be done inside the RAII holder.
	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	RAIIParamsHolder ParamAlloc(*Function, Parms);

	TSet<FUnrealObjectRef> UnresolvedRefs;
	{
		// Scope for the SpatialNetBitReader lifecycle (only one should exist per thread).
		TSet<FUnrealObjectRef> MappedRefs;

		FSpatialNetBitReader PayloadReader(NetDriver->PackageMap, const_cast<uint8*>(RPCData.PayloadData.GetData()),
										   RPCData.PayloadData.Num() * 8, MappedRefs, UnresolvedRefs);

		TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
		RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);
	}

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	const float TimeQueued = (FPlatformTime::Cycles64() - MetaData.Timestamp) * FPlatformTime::GetSecondsPerCycle64();
	const int32 UnresolvedRefCount = UnresolvedRefs.Num();

	if (UnresolvedRefCount != 0 && TimeQueued < SpatialSettings->QueuedIncomingRPCWaitTime)
	{
		return !RPCConsumed;
	}

	if (UnresolvedRefCount > 0
		// -> Use MetaData.ReceiverName to recover the RPC type.
		/* && !SpatialSettings->ShouldRPCTypeAllowUnresolvedParameters(PendingRPCParams.Type)*/
		&& (Function->SpatialFunctionFlags & SPATIALFUNC_AllowUnresolvedParameters) == 0)
	{
		const FString UnresolvedEntityIds = FString::JoinBy(UnresolvedRefs, TEXT(", "), [](const FUnrealObjectRef& Ref) {
			return Ref.ToString();
		});

		//UE_LOG(LogSpatialRPCService, Warning,
		//	   TEXT("Executed RPC %s::%s with unresolved references (%s) after %.3f seconds of queueing. Owner name: %s"),
		//	   *GetNameSafe(TargetObject), *GetNameSafe(Function), *UnresolvedEntityIds, TimeQueued,
		//	   *GetNameSafe(TargetObject->GetOuter()));
	}

	// Get the RPC target Actor (was used for additional debug authority checking, but CanExtractRPC should handle that now)
	// AActor* Actor = TargetObject->IsA<AActor>() ? Cast<AActor>(TargetObject) : TargetObject->GetTypedOuter<AActor>();

	const bool bUseEventTracer = EventTracer != nullptr;
	if (bUseEventTracer)
	{
		FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateApplyRPC(TargetObject, Function),
														   /* Causes */ MetaData.SpanId.GetConstId(), /* NumCauses */ 1);
		EventTracer->AddToStack(SpanId);
	}

	TargetObject->ProcessEvent(Function, Parms);

	if (bUseEventTracer)
	{
		EventTracer->PopFromStack();
	}

	return RPCConsumed;
}

void USpatialNetDriverRPC::MakeRingBufferWithACKSender(FName QueueName, ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
													   TUniquePtr<RPCBufferSender>& SenderPtr,
													   TUniquePtr<TRPCQueue<FRPCPayload, FSpatialGDKSpanId>>& QueuePtr)
{
	// TODO UNR-5037
}

void USpatialNetDriverRPC::MakeRingBufferWithACKReceiver(FName ReceiverName, ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
														 TUniquePtr<TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>>& ReceiverPtr)
{
	// TODO UNR-5037
}

USpatialNetDriverServerRPC::USpatialNetDriverServerRPC() {}

void USpatialNetDriverServerRPC::FlushRPCUpdates()
{
	USpatialNetDriverRPC::FlushRPCUpdates();
	FlushRPCQueue(*ClientReliableQueue);
	FlushRPCQueue(*ClientUnreliableQueue);
}

void USpatialNetDriverServerRPC::ProcessReceivedRPCs()
{
	USpatialNetDriverRPC::ProcessReceivedRPCs();

	auto CanExtractRPCs = [this](Worker_EntityId EntityId) {
		return CanExtractRPCOnServer(EntityId);
	};
	auto ProcessRPC = [this](Worker_EntityId EntityId, ReceivedRPC RPCData, const FRPCMetaData& MetaData) {
		return ApplyRPC(EntityId, RPCData, MetaData);
	};

	ServerReliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
	ServerUnreliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
}

void USpatialNetDriverServerRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	USpatialNetDriverRPC::GetRPCComponentsOnEntityCreation(EntityId, OutData);

	{
		RPCWritingContext Ctx(ClientReliableQueue->Name, MakeDataWriteCallback(OutData));
		ClientReliableQueue->Flush(EntityId, Ctx, StandardQueue::SentRPCCallback(), /*bIgnoreAdded*/ true);
	}
	{
		RPCWritingContext Ctx(ClientUnreliableQueue->Name, MakeDataWriteCallback(OutData));
		ClientUnreliableQueue->Flush(EntityId, Ctx, StandardQueue::SentRPCCallback(), /*bIgnoreAdded*/ true);
	}
}

void USpatialNetDriverServerRPC::Init(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
									  const SpatialGDK::FSubView& InActorNonAuthSubView)
{
	USpatialNetDriverRPC::Init(InNetDriver, InActorAuthSubView, InActorNonAuthSubView);
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();

	auto constexpr RequiredAuth = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;

	MakeRingBufferWithACKSender("ClientReliable", ERPCType::ClientReliable, RequiredAuth, ClientReliableSender, ClientReliableQueue);
	MakeRingBufferWithACKSender("ClientUnreliable", ERPCType::ClientUnreliable, RequiredAuth, ClientUnreliableSender,
								ClientUnreliableQueue);

	MakeRingBufferWithACKReceiver("ServerReliable", ERPCType::ServerReliable, RequiredAuth, ServerReliableReceiver);
	MakeRingBufferWithACKReceiver("ServerUnreliable", ERPCType::ServerUnreliable, RequiredAuth, ServerUnreliableReceiver);

	{
		auto RPCType = ERPCType::NetMulticast;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		// TODO
	}
}

USpatialNetDriverClientRPC::USpatialNetDriverClientRPC() {}

void USpatialNetDriverClientRPC::FlushRPCUpdates()
{
	USpatialNetDriverRPC::FlushRPCUpdates();
	FlushRPCQueue(*ServerReliableQueue);
	FlushRPCQueue(*ServerUnreliableQueue);
}

void USpatialNetDriverClientRPC::ProcessReceivedRPCs()
{
	USpatialNetDriverRPC::ProcessReceivedRPCs();

	auto CanExtractRPCs = [this](Worker_EntityId EntityId) {
		return CanExtractRPC(EntityId);
	};
	auto ProcessRPC = [this](Worker_EntityId EntityId, ReceivedRPC RPCData, const FRPCMetaData& MetaData) {
		return ApplyRPC(EntityId, RPCData, MetaData);
	};

	ClientReliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
	ClientUnreliableReceiver->ExtractReceivedRPCs(CanExtractRPCs, ProcessRPC);
}

void USpatialNetDriverClientRPC::GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData)
{
	// Clients should not flush anything....
	checkNoEntry();
}

void USpatialNetDriverClientRPC::Init(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
									  const SpatialGDK::FSubView& InActorNonAuthSubView)
{
	USpatialNetDriverRPC::Init(InNetDriver, InActorAuthSubView, InActorNonAuthSubView);

	auto constexpr RequiredAuth = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;

	MakeRingBufferWithACKSender("ServerReliable", ERPCType::ServerReliable, RequiredAuth, ServerReliableSender, ServerReliableQueue);
	MakeRingBufferWithACKSender("ServerUnreliable", ERPCType::ServerUnreliable, RequiredAuth, ServerUnreliableSender,
								ServerUnreliableQueue);

	MakeRingBufferWithACKReceiver("ClientReliable", ERPCType::ClientReliable, RequiredAuth, ClientReliableReceiver);
	MakeRingBufferWithACKReceiver("ClientUnreliable", ERPCType::ClientUnreliable, RequiredAuth, ClientUnreliableReceiver);
}
