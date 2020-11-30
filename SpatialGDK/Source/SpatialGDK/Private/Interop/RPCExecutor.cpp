// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCExecutor.h"

#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Utils/RepLayoutUtils.h"

namespace SpatialGDK
{
RPCExecutor::RPCExecutor(class USpatialNetDriver* InNetDriver)
	: NetDriver(InNetDriver)
{
}

bool RPCExecutor::ExecuteCommand(const FCrossServerRPCParams& Params)
{
	const TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(Params.ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return false;
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObjectWeakPtr.Get());
	UFunction* Function = ClassInfo.RPCs[Params.Payload.Index];
	if (Function == nullptr)
	{
		return true;
	}

	uint8* Parms = static_cast<uint8*>(FMemory_Alloca(Function->ParmsSize));
	FMemory::Memzero(Parms, Function->ParmsSize);

	TSet<FUnrealObjectRef> UnresolvedRefs;
	TSet<FUnrealObjectRef> MappedRefs;
	RPCPayload PayloadCopy = Params.Payload;
	FSpatialNetBitReader PayloadReader(NetDriver->PackageMap, PayloadCopy.PayloadData.GetData(), PayloadCopy.CountDataBits(), MappedRefs,
									   UnresolvedRefs);

	TSharedPtr<FRepLayout> RepLayout = NetDriver->GetFunctionRepLayout(Function);
	RepLayout_ReceivePropertiesForRPC(*RepLayout, PayloadReader, Parms);

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	const float TimeQueued = (FDateTime::Now() - Params.Timestamp).GetTotalSeconds();
	bool CanProcessRPC = UnresolvedRefs.Num() == 0 || SpatialSettings->QueuedIncomingRPCWaitTime < TimeQueued;

	if (CanProcessRPC)
	{
		TargetObject->ProcessEvent(Function, Parms);
	}

	// Destroy the parameters.
	// warning: highly dependent on UObject::ProcessEvent freeing of parms!
	for (TFieldIterator<GDK_PROPERTY(Property)> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms);
	}

	return CanProcessRPC;
}

FCrossServerRPCParams RPCExecutor::CreateInvalidParams()
{
	return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, 0, {} };
}

FCrossServerRPCParams RPCExecutor::TryRetrieveCrossServerRPCParams(const Worker_Op& Op)
{
	if (NetDriver->Receiver->IsEntityWaitingForAsyncLoad(Op.op.command_request.entity_id))
	{
		return CreateInvalidParams();
	}

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Op.op.command_request.request.schema_type);
	RPCPayload Payload(RequestObject);
	const FUnrealObjectRef ObjectRef = FUnrealObjectRef(Op.op.command_request.entity_id, Payload.Offset);
	const TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return CreateInvalidParams();
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);

	if (Payload.Index >= static_cast<uint32>(ClassInfo.RPCs.Num()))
	{
		// This should only happen if there's a class layout disagreement between workers, which would indicate incompatible binaries.
		return CreateInvalidParams();
	}

	UFunction* Function = ClassInfo.RPCs[Payload.Index];
	if (Function == nullptr)
	{
		return CreateInvalidParams();
	}

	const auto RPCInfo = NetDriver->ClassInfoManager->GetRPCInfo(TargetObject, Function);
	if (RPCInfo.Type != ERPCType::CrossServer)
	{
		return CreateInvalidParams();
	}

	AActor* TargetActor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(Op.op.command_request.entity_id));
#if TRACE_LIB_ACTIVE
	TraceKey TraceId = Payload.Trace;
#else
	TraceKey TraceId = InvalidTraceKey;
#endif

	UObject* TraceTargetObject = TargetActor != TargetObject ? TargetObject : nullptr;
	FSpatialGDKSpanId SpanId = NetDriver->Connection->GetEventTracer()->TraceEvent(FSpatialTraceEventBuilder::CreateReceiveCommandRequest(
		"RPC_COMMAND_REQUEST", TargetActor, TraceTargetObject, Function, TraceId, Op.op.command_request.request_id));
	return { ObjectRef, Op.op.command_request.request_id, MoveTemp(Payload), Op.op.command_request.timeout_millis, SpanId };
}
} // namespace SpatialGDK
