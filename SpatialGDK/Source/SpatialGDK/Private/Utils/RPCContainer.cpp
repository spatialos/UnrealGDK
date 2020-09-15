// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCContainer.h"

#include "Schema/UnrealObjectRef.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogRPCContainer);

using namespace SpatialGDK;

namespace
{
FString ERPCResultToString(ERPCResult Result)
{
	switch (Result)
	{
	case ERPCResult::Success:
		return TEXT("");

	case ERPCResult::UnresolvedTargetObject:
		return TEXT("Unresolved Target Object");

	case ERPCResult::MissingFunctionInfo:
		return TEXT("Missing UFunction info");

	case ERPCResult::UnresolvedParameters:
		return TEXT("Unresolved Parameters");

	case ERPCResult::NoActorChannel:
		return TEXT("No Actor Channel");

	case ERPCResult::SpatialActorChannelNotListening:
		return TEXT("Spatial Actor Channel Not Listening");

	case ERPCResult::NoNetConnection:
		return TEXT("No Net Connection");

	case ERPCResult::NoAuthority:
		return TEXT("No Authority");

	case ERPCResult::InvalidRPCType:
		return TEXT("Invalid RPC Type");

	case ERPCResult::NoOwningController:
		return TEXT("No Owning Controller");

	case ERPCResult::NoControllerChannel:
		return TEXT("No Controller Channel");

	case ERPCResult::ControllerChannelNotListening:
		return TEXT("Controller Channel Not Listening");

	case ERPCResult::RPCServiceFailure:
		return TEXT("SpatialRPCService couldn't handle the RPC");

	default:
		return TEXT("Unknown");
	}
}

void LogRPCError(const FRPCErrorInfo& ErrorInfo, ERPCQueueType QueueType, const FPendingRPCParams& Params)
{
	const FTimespan TimeDiff = FDateTime::Now() - Params.Timestamp;

	// The format is expected to be:
	// Function <objectName>::<functionName> sending/execution dropped/queued for <duration>. Reason: <reason>
	FString OutputLog = FString::Printf(
		TEXT("Function %s::%s %s %s for %s. Reason: %s"),
		ErrorInfo.TargetObject.IsValid() ? *ErrorInfo.TargetObject->GetName() : TEXT("UNKNOWN"),
		ErrorInfo.Function.IsValid() ? *ErrorInfo.Function->GetName() : TEXT("UNKNOWN"),
		QueueType == ERPCQueueType::Send ? TEXT("sending") : QueueType == ERPCQueueType::Receive ? TEXT("execution") : TEXT("UNKNOWN"),
		ErrorInfo.QueueProcessResult == ERPCQueueProcessResult::ContinueProcessing ? TEXT("queued") : TEXT("dropped"), *TimeDiff.ToString(),
		*ERPCResultToString(ErrorInfo.ErrorCode));

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	check(SpatialGDKSettings != nullptr);

	if (TimeDiff.GetTotalSeconds() > SpatialGDKSettings->GetSecondsBeforeWarning(ErrorInfo.ErrorCode))
	{
		UE_LOG(LogRPCContainer, Warning, TEXT("%s"), *OutputLog);
	}
	else
	{
		UE_LOG(LogRPCContainer, Verbose, TEXT("%s"), *OutputLog);
	}
}
} // namespace

// FPendingRPCParams::FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, const FUnrealObjectRef& InSenderObjectRef, ERPCType
// InType, RPCPayload&& InPayload) 	: ObjectRef(InTargetObjectRef) 	, SenderObjectRef(InSenderObjectRef) 	, Payload(MoveTemp(InPayload)) 	,
//Timestamp(FDateTime::Now()) 	, Type(InType)
//{
//}

FPendingRPCParams::FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, const FUnrealObjectRef& InSenderObjectRef, ERPCType InType,
									 RPCPayload&& InPayload, uint32 InSlot)
	: ObjectRef(InTargetObjectRef)
	, SenderObjectRef(InSenderObjectRef)
	, Payload(MoveTemp(InPayload))
	, Timestamp(FDateTime::Now())
	, Type(InType)
	, Slot(InSlot)
{
}

void FRPCContainer::ProcessOrQueueRPC(const FUnrealObjectRef& TargetObjectRef, const FUnrealObjectRef& SenderObjectRef, ERPCType Type,
									  RPCPayload&& Payload, uint32 Slot)
{
	FArrayOfParams& ArrayOfParams = QueuedRPCs.FindOrAdd(Type).FindOrAdd(TargetObjectRef.Entity);

	if (Type == ERPCType::CrossServerSender)
	{
		for (auto const& Entry : ArrayOfParams)
		{
			if (Entry.SenderObjectRef == SenderObjectRef)
			{
				// Already queued RPC, can happen while reading RPC over several frames.
				return;
			}
		}
	}

	FPendingRPCParams Params{ TargetObjectRef, SenderObjectRef, Type, MoveTemp(Payload), Slot };

	if (!ObjectHasRPCsQueuedOfType(Params.ObjectRef.Entity, Params.Type))
	{
		const ERPCQueueProcessResult QueueProcessResult = ApplyFunction(Params);
		switch (QueueProcessResult)
		{
		case ERPCQueueProcessResult::ContinueProcessing:
		case ERPCQueueProcessResult::DropEntireQueue:
			return;
		}
	}

	ArrayOfParams.Push(MoveTemp(Params));
}

void FRPCContainer::ProcessRPCs(FArrayOfParams& RPCList)
{
	// TODO: UNR-1651 Find a way to drop queued RPCs
	int NumProcessedParams = 0;
	for (auto& Params : RPCList)
	{
		const ERPCQueueProcessResult QueueProcessResult = ApplyFunction(Params);
		switch (QueueProcessResult)
		{
		case ERPCQueueProcessResult::ContinueProcessing:
			NumProcessedParams++;
			break;
		case ERPCQueueProcessResult::StopProcessing:
			// The actual break out of the for loop is handled below
			break;
		case ERPCQueueProcessResult::DropEntireQueue:
			RPCList.Empty();
			return;
		}

		if (QueueProcessResult == ERPCQueueProcessResult::StopProcessing)
		{
			break;
		}
	}

	RPCList.RemoveAt(0, NumProcessedParams);
}

void FRPCContainer::ProcessRPCs()
{
	if (bAlreadyProcessingRPCs)
	{
		UE_LOG(LogRPCContainer, Log, TEXT("Calling ProcessRPCs recursively, ignoring the call"));
		return;
	}

	bAlreadyProcessingRPCs = true;

	for (auto& RPCs : QueuedRPCs)
	{
		FRPCMap& MapOfQueues = RPCs.Value;
		for (auto It = MapOfQueues.CreateIterator(); It; ++It)
		{
			FArrayOfParams& RPCList = It.Value();
			ProcessRPCs(RPCList);
			if (RPCList.Num() == 0)
			{
				It.RemoveCurrent();
			}
		}
	}

	bAlreadyProcessingRPCs = false;
}

void FRPCContainer::DropForEntity(const Worker_EntityId& EntityId)
{
	for (auto& RpcMap : QueuedRPCs)
	{
		RpcMap.Value.Remove(EntityId);
	}
}

bool FRPCContainer::ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ERPCType Type) const
{
	if (const FRPCMap* MapOfQueues = QueuedRPCs.Find(Type))
	{
		if (const FArrayOfParams* RPCList = MapOfQueues->Find(EntityId))
		{
			return (RPCList->Num() > 0);
		}
	}

	return false;
}

FRPCContainer::FRPCContainer(ERPCQueueType InQueueType)
	: QueueType(InQueueType)
{
}

void FRPCContainer::BindProcessingFunction(const FProcessRPCDelegate& Function)
{
	ProcessingFunction = Function;
}

ERPCQueueProcessResult FRPCContainer::ApplyFunction(FPendingRPCParams& Params)
{
	ensure(ProcessingFunction.IsBound());
	FRPCErrorInfo ErrorInfo = ProcessingFunction.Execute(Params);

	if (ErrorInfo.Success())
	{
		return ERPCQueueProcessResult::ContinueProcessing;
	}

#if !UE_BUILD_SHIPPING
	LogRPCError(ErrorInfo, QueueType, Params);
#endif

	return ErrorInfo.QueueProcessResult;
}
