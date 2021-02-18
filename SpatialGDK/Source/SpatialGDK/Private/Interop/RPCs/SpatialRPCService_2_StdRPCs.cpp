#include "Interop/RPCs/SpatialRPCService_2_StdRPCs.h"
#include "Interop/RPCs/SpatialRPCService_2_Queues.h"
#include "Interop/RPCs/SpatialRPCService_2_Receivers.h"
#include "Interop/RPCs/SpatialRPCService_2_Senders.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

namespace SpatialGDK
{
ServerStdRPCs::ServerStdRPCs(SpatialRPCService_2& Service)
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	{
		auto RPCType = ERPCType::ClientReliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		SpatialRPCService_2::RPCQueueDescription Desc;
		auto Sender = MakeShared<SchemaMonotonicRingBufferWithACKSender>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		Desc.Sender = Sender;
		ClientReliableRPCQueue = MakeShared<TRPCLocalOverflowQueue<RPCPayload>>(*Sender);
		Desc.Queue = ClientReliableRPCQueue;
		Desc.Authority = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;

		Service.AddRPCQueue("ClientReliableQueue", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::ClientUnreliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		SpatialRPCService_2::RPCQueueDescription Desc;
		auto Sender = MakeShared<SchemaMonotonicRingBufferWithACKSender>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));
		Desc.Sender = Sender;

		ClientUnreliableRPCQueue = MakeShared<TRPCFixedCapacityQueue<RPCPayload>>(*Sender, RPCDesc.RingBufferSize);
		Desc.Queue = ClientUnreliableRPCQueue;
		Desc.Authority = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;

		Service.AddRPCQueue("ClientUnreliableQueue", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::NetMulticast;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		{
			SpatialRPCService_2::RPCQueueDescription Desc;
			auto Sender =
				MakeShared<SchemaMonotonicRingBufferSender>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
															RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart, RPCDesc.RingBufferSize);
			Desc.Sender = Sender;

			MulticastRPCQueue = MakeShared<TRPCFixedCapacityQueue<RPCPayload>>(*Sender, RPCDesc.RingBufferSize);
			Desc.Queue = MulticastRPCQueue;
			Desc.Authority = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;

			Service.AddRPCQueue("NetMulticastQueue", MoveTemp(Desc));
		}

		{
			// MulticastReceiver = MakeShared<SchemaMonotonicRingBufferReceiver>(RPCRingBufferUtils::GetRingBufferComponentId(RPCType),
			//	RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			//	RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType),
			//	RPCRingBufferUtils::GetAckFieldId(RPCType));
			//
			// SpatialRPCService_2::RPCReceiverDescription Desc;
			// Desc.Authority = 0;
			// Desc.Receiver = ServerReliableReceiver;
			//
			// Service.AddRPCReceiver("ServerReliableReceiver", MoveTemp(Desc));
		}
	}

	{
		auto RPCType = ERPCType::ServerReliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		ServerReliableReceiver = MakeShared<SchemaMonotonicRingBufferWithACKReceiver>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		SpatialRPCService_2::RPCReceiverDescription Desc;
		Desc.Authority = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;
		Desc.Receiver = ServerReliableReceiver;

		Service.AddRPCReceiver("ServerReliableReceiver", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::ServerUnreliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		ServerUnreliableReceiver = MakeShared<SchemaMonotonicRingBufferWithACKReceiver>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		SpatialRPCService_2::RPCReceiverDescription Desc;
		Desc.Authority = SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;
		Desc.Receiver = ServerUnreliableReceiver;

		Service.AddRPCReceiver("ServerUnreliableReceiver", MoveTemp(Desc));
	}
}

ClientStdRPCs::ClientStdRPCs(SpatialRPCService_2& Service)
{
	{
		auto RPCType = ERPCType::ServerReliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		SpatialRPCService_2::RPCQueueDescription Desc;
		auto Sender = MakeShared<SchemaMonotonicRingBufferWithACKSender>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		Desc.Sender = Sender;
		ServerReliableRPCQueue = MakeShared<TRPCLocalOverflowQueue<RPCPayload>>(*Sender);
		Desc.Queue = ServerReliableRPCQueue;
		Desc.Authority = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;

		Service.AddRPCQueue("ServerReliableQueue", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::ServerUnreliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		SpatialRPCService_2::RPCQueueDescription Desc;
		auto Sender = MakeShared<SchemaMonotonicRingBufferWithACKSender>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		Desc.Sender = Sender;
		ServerUnreliableRPCQueue = MakeShared<TRPCFixedCapacityQueue<RPCPayload>>(*Sender, RPCDesc.RingBufferSize);
		Desc.Queue = ServerUnreliableRPCQueue;
		Desc.Authority = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;

		Service.AddRPCQueue("ServerUnreliableQueue", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::ClientReliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		ClientReliableReceiver = MakeShared<SchemaMonotonicRingBufferWithACKReceiver>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		SpatialRPCService_2::RPCReceiverDescription Desc;
		Desc.Authority = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;
		Desc.Receiver = ClientReliableReceiver;

		Service.AddRPCReceiver("ClientReliableReceiver", MoveTemp(Desc));
	}

	{
		auto RPCType = ERPCType::ClientUnreliable;
		auto RPCDesc = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);

		ClientUnreliableReceiver = MakeShared<SchemaMonotonicRingBufferWithACKReceiver>(
			RPCRingBufferUtils::GetRingBufferComponentId(RPCType), RPCDesc.LastSentRPCFieldId, RPCDesc.SchemaFieldStart,
			RPCDesc.RingBufferSize, RPCRingBufferUtils::GetAckComponentId(RPCType), RPCRingBufferUtils::GetAckFieldId(RPCType));

		SpatialRPCService_2::RPCReceiverDescription Desc;
		Desc.Authority = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;
		Desc.Receiver = ClientUnreliableReceiver;

		Service.AddRPCReceiver("ClientUnreliableReceiver", MoveTemp(Desc));
	}

	{
		Worker_ComponentId MovementChannelComp = 0;
		Schema_FieldId RPCField = 1;
		Schema_FieldId CountField = 2;
		uint32_t BufferSize = 1;

		SpatialRPCService_2::RPCQueueDescription Desc;
		auto Sender = MakeShared<SchemaMonotonicRingBufferSender>(MovementChannelComp, RPCField, CountField, BufferSize);

		Desc.Sender = Sender;
		NetMovementRPCQueue = MakeShared<TRPCMostRecentQueue<RPCPayload>>(*Sender, BufferSize);
		Desc.Queue = NetMovementRPCQueue;
		Desc.Authority = SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;

		Service.AddRPCQueue("MovementChannelQueue", MoveTemp(Desc));
	}
}
} // namespace SpatialGDK

#include "EngineClasses/SpatialNetBitReader.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/RepLayoutUtils.h"

namespace SpatialGDK
{
FRPCErrorInfo ApplyRPCInternal(USpatialNetDriver* NetDriver, UObject* TargetObject, UFunction* Function,
							   const FPendingRPCParams& PendingRPCParams)
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
			// bool bUseEventTracer = EventTracer != nullptr && RPCType != ERPCType::CrossServer;
			// if (bUseEventTracer)
			//{
			//	FSpatialGDKSpanId SpanId = EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateApplyRPC(TargetObject, Function),
			//		/* Causes */ PendingRPCParams.SpanId.GetConstId(), /* NumCauses */ 1);
			//	EventTracer->AddToStack(SpanId);
			//}

			TargetObject->ProcessEvent(Function, Parms);

			// if (bUseEventTracer)
			//{
			//	EventTracer->PopFromStack();
			//}

			// if (RPCType == ERPCType::CrossServer)
			//{
			//	if (CrossServerRPCs && PendingRPCParams.SenderRPCInfo.Entity != SpatialConstants::INVALID_ENTITY_ID)
			//	{
			//		CrossServerRPCs->WriteCrossServerACKFor(PendingRPCParams.ObjectRef.Entity, PendingRPCParams.SenderRPCInfo);
			//	}
			//}
			// else if (RPCType != ERPCType::NetMulticast)
			//{
			//	ClientServerRPCs.IncrementAckedRPCID(PendingRPCParams.ObjectRef.Entity, RPCType);
			//}

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

FRPCErrorInfo ApplyRPC(USpatialNetDriver* NetDriver, const FPendingRPCParams& Params)
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

	return ApplyRPCInternal(NetDriver, TargetObject, Function, Params);
}

bool CanExtractRPC(USpatialNetDriver* NetDriver, Worker_EntityId EntityId)
{
	const TWeakObjectPtr<UObject> ActorReceivingRPC = NetDriver->PackageMap->GetObjectFromEntityId(EntityId);
	if (!ActorReceivingRPC.IsValid())
	{
		UE_LOG(LogSpatialRPCService, Log,
			   TEXT("Entity receiving ring buffer RPC does not exist in PackageMap, possibly due to corresponding actor getting "
					"destroyed. Entity: %lld"),
			   EntityId);
		return false;
	}

	const bool bActorRoleIsSimulatedProxy = Cast<AActor>(ActorReceivingRPC.Get())->Role == ROLE_SimulatedProxy;
	if (bActorRoleIsSimulatedProxy)
	{
		UE_LOG(LogSpatialRPCService, Verbose,
			   TEXT("Will not process server RPC, Actor role changed to SimulatedProxy. This happens on migration. Entity: %lld"),
			   EntityId);
		return false;
	}
	return true;
}

} // namespace SpatialGDK

void ExecuteIncomingRPC(USpatialNetDriver* NetDriver, SpatialGDK::RPCBufferReceiver& Receiver)
{
	Receiver.ExtractReceivedRPCs(
		[NetDriver](Worker_EntityId EntityId) {
			return SpatialGDK::CanExtractRPC(NetDriver, EntityId);
		},
		[NetDriver](const FUnrealObjectRef& Object, const SpatialGDK::RPCPayload& Payload) {
			FPendingRPCParams pendingRPC(Object, SpatialGDK::RPCSender(), ERPCType::Invalid, SpatialGDK::RPCPayload(Payload), {});
			return SpatialGDK::ApplyRPC(NetDriver, pendingRPC).ErrorCode == ERPCResult::Success;
		});
}
