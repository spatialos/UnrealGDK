// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCExecutor.h"

#include "Interop/SpatialPlayerSpawner.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Utils/RepLayoutUtils.h"

using namespace SpatialGDK;

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

FCrossServerRPCParams RPCExecutor::TryRetrieveCrossServerRPCParams(const Worker_Op& Op)
{
	if (NetDriver->Receiver->IsEntityWaitingForAsyncLoad(Op.op.command_request.entity_id))
	{
		return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, {} };
	}

	Schema_Object* RequestObject = Schema_GetCommandRequestObject(Op.op.command_request.request.schema_type);
	RPCPayload Payload(RequestObject);
	const FUnrealObjectRef ObjectRef = FUnrealObjectRef(Op.op.command_request.entity_id, Payload.Offset);
	const TWeakObjectPtr<UObject> TargetObjectWeakPtr = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(ObjectRef);
	if (!TargetObjectWeakPtr.IsValid())
	{
		return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, {} };
	}

	UObject* TargetObject = TargetObjectWeakPtr.Get();
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(TargetObject);

	if (Payload.Index >= static_cast<uint32>(ClassInfo.RPCs.Num()))
	{
		// This should only happen if there's a class layout disagreement between workers, which would indicate incompatible binaries.
		return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, {} };
	}

	UFunction* Function = ClassInfo.RPCs[Payload.Index];
	if (Function == nullptr)
	{
		return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, {} };
	}

	const auto RPCInfo = NetDriver->ClassInfoManager->GetRPCInfo(TargetObject, Function);
	if (RPCInfo.Type != ERPCType::CrossServer)
	{
		return { FUnrealObjectRef(), -1, { 0, 0, 0, {} }, {} };
	}
	return { ObjectRef, Op.op.command_request.request_id, MoveTemp(Payload), Op.span_id };
}
