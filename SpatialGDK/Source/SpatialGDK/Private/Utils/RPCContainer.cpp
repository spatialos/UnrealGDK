// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCContainer.h"

#include "Schema/UnrealObjectRef.h"

DEFINE_LOG_CATEGORY(LogRPCContainer);

using namespace SpatialGDK;

namespace
{
	FString ERPCErrorToString(ERPCError Error)
	{
		switch (Error)
		{
		case ERPCError::Success:
			return TEXT("");

		case ERPCError::UnresolvedTargetObject:
			return TEXT("Unresolved Target Object");

		case ERPCError::MissingFunctionInfo:
			return TEXT("Missing UFunction info");

		case ERPCError::UnresolvedParameters:
			return TEXT("Unresolved Parameters");

		case ERPCError::NoActorChannel:
			return TEXT("No Actor Channel");

		case ERPCError::SpatialActorChannelNotListening:
			return TEXT("Spatial Actor Channel Not Listening");

		case ERPCError::NoNetConnection:
			return TEXT("No Net Connection");

		case ERPCError::NoAuthority:
			return TEXT("No Authority");

		case ERPCError::InvalidRPCType:
			return TEXT("Invalid RPC Type");

		case ERPCError::NoOwningController:
			return TEXT("No Owning Controller");

		case ERPCError::NoControllerChannel:
			return TEXT("No Controller Channel");

		case ERPCError::UnresolvedController:
			return TEXT("Unresolved Controller");

		case ERPCError::ControllerChannelNotListening:
			return TEXT("Controller Channel Not Listening");

		default:
			return TEXT("");
		}
	}

	void LogRPCError(const FRPCErrorInfo& ErrorInfo, const FPendingRPCParams& Params, bool DroppingRPC = false)
	{
		FTimespan TimeDiff = FDateTime::Now() - Params.Timestamp;
		if (ErrorInfo.TargetObject == nullptr)
		{
			UE_LOG(LogRPCContainer, Warning, TEXT("Function UNKNOWN::UNKNOWN queued for %s Reason: %s"), *ErrorInfo.TargetObject->GetName(), *TimeDiff.ToString(), *ERPCErrorToString(ErrorInfo.ErrorCode));
		}
		else
		{
			if (ErrorInfo.Function == nullptr)
			{
				UE_LOG(LogRPCContainer, Warning, TEXT("Function %s::UNKNOWN queued for %s Reason: %s"), *ErrorInfo.TargetObject->GetName(), *TimeDiff.ToString(), *ERPCErrorToString(ErrorInfo.ErrorCode));
			}
			else
			{
				UE_LOG(LogRPCContainer, Warning, TEXT("Function %s::%s queued for %s Reason: %s"), *ErrorInfo.TargetObject->GetName(), *ErrorInfo.Function->GetName(), *TimeDiff.ToString(), *ERPCErrorToString(ErrorInfo.ErrorCode));
			}
		}

		if (DroppingRPC)
		{
			UE_LOG(LogRPCContainer, Error, TEXT("Dropping the RPC"));
		}
	}
}

FPendingRPCParams::FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, ESchemaComponentType InType, RPCPayload&& InPayload)
	: ObjectRef(InTargetObjectRef)
	, Payload(MoveTemp(InPayload))
	, Timestamp(FDateTime::Now())
	, Type(InType)
{
}

void FRPCContainer::ProcessOrQueueRPC(const FUnrealObjectRef& TargetObjectRef, ESchemaComponentType Type, RPCPayload&& Payload)
{
	FPendingRPCParams Params {TargetObjectRef, Type, MoveTemp(Payload)};

	if (!ObjectHasRPCsQueuedOfType(Params.ObjectRef.Entity, Params.Type))
	{
		if (ApplyFunction(Params))
		{
			return;
		}
	}

	FArrayOfParams& ArrayOfParams = QueuedRPCs.FindOrAdd(Params.Type).FindOrAdd(Params.ObjectRef.Entity);
	ArrayOfParams.Push(MoveTemp(Params));
}

void FRPCContainer::ProcessRPCs(FArrayOfParams& RPCList)
{
	// TODO: UNR-1651 Find a way to drop queued RPCs
	int NumProcessedParams = 0;
	for (auto& Params : RPCList)
	{
		if (ApplyFunction(Params))
		{
			NumProcessedParams++;
		}
		else
		{
			break;
		}
	}
	RPCList.RemoveAt(0, NumProcessedParams);
}

void FRPCContainer::ProcessRPCs()
{
	for (auto& RPCs : QueuedRPCs)
	{
		FRPCMap& MapOfQueues = RPCs.Value;
		for(auto It = MapOfQueues.CreateIterator(); It; ++It)
		{
			FArrayOfParams& RPCList = It.Value();
			ProcessRPCs(RPCList);
			if (RPCList.Num() == 0)
			{
				It.RemoveCurrent();
			}
		}
	}
}

bool FRPCContainer::ObjectHasRPCsQueuedOfType(const Worker_EntityId& EntityId, ESchemaComponentType Type) const
{
	if(const FRPCMap* MapOfQueues = QueuedRPCs.Find(Type))
	{
		if(const FArrayOfParams* RPCList = MapOfQueues->Find(EntityId))
		{
			return (RPCList->Num() > 0);
		}
	}

	return false;
}
 
void FRPCContainer::BindProcessingFunction(const FProcessRPCDelegate& Function)
{
	ProcessingFunction = Function;
}

bool FRPCContainer::ApplyFunction(FPendingRPCParams& Params)
{
	check(ProcessingFunction.IsBound());
	FRPCErrorInfo ErrorInfo = ProcessingFunction.Execute(Params);
	if (ErrorInfo.Success())
	{
		return true;
	}
	else
	{
		FTimespan TimeDiff = FDateTime::Now() - Params.Timestamp;
		if (TimeDiff.GetSeconds() > SECONDS_TO_DROP_RPC)
		{
			LogRPCError(ErrorInfo, Params, true);
			return true;
		}
		else
		{
			LogRPCError(ErrorInfo, Params, false);
			return false;
		}
	}
}
