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
			return "";

		case ERPCError::UnresolvedTargetObject:
			return "Unresolved Target Object";

		case ERPCError::MissingFunctionInfo:
			return "Missing UFunction info";

		case ERPCError::UnresolvedParameters:
			return "Unresolved Parameters";

		case ERPCError::NoActorChannel:
			return "No Actor Channel";

		case ERPCError::SpatialActorChannelNotListening:
			return "Spatial Actor Channel Not Listening";

		case ERPCError::NoNetConnection:
			return "No Net Connection";

		case ERPCError::NoAuthority:
			return "No Authority";

		case ERPCError::InvalidRPCType:
			return "Invalid RPC Type";

		case ERPCError::NoOwningController:
			return "No Owning Controller";

		case ERPCError::NoControllerChannel:
			return "No Controller Channel";

		case ERPCError::UnresolvedController:
			return "Unresolved Controller";

		case ERPCError::ControllerChannelNotListening:
			return "Controller Channel Not Listening";

		default:
			return "";
		}
	}

	void LogRPCError(const FRPCErrorInfo& ErrorInfo, const FPendingRPCParams& Params)
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
	}
}

FPendingRPCParams::FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload, int InReliableRPCIndex /* = 0 */)
	: ReliableRPCIndex(InReliableRPCIndex)
	, ObjectRef(InTargetObjectRef)
	, Payload(MoveTemp(InPayload))
	, Timestamp(FDateTime::Now())
{
}

void FRPCContainer::QueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type)
{
	FArrayOfParams& ArrayOfParams = QueuedRPCs.FindOrAdd(Type).FindOrAdd(Params->ObjectRef.Entity);
	ArrayOfParams.Push(MoveTemp(Params));
}

void FRPCContainer::ProcessRPCs(const FProcessRPCDelegate& FunctionToApply, FArrayOfParams& RPCList)
{
	// TODO: UNR-1651 Find a way to drop queued RPCs
	int NumProcessedParams = 0;
	for (auto& Params : RPCList)
	{
		if (ApplyFunction(FunctionToApply, *Params))
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

void FRPCContainer::ProcessRPCs(const FProcessRPCDelegate& FunctionToApply)
{
	for (auto& RPCs : QueuedRPCs)
	{
		FRPCMap& MapOfQueues = RPCs.Value;
		for(auto It = MapOfQueues.CreateIterator(); It; ++It)
		{
			FArrayOfParams& RPCList = It.Value();
			ProcessRPCs(FunctionToApply, RPCList);
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

bool FRPCContainer::ApplyFunction(const FProcessRPCDelegate& FunctionToApply, const FPendingRPCParams& Params)
{
	FRPCErrorInfo ErrorInfo = FunctionToApply.Execute(Params);
	if (ErrorInfo.Success())
	{
		return true;
	}
	else
	{
		LogRPCError(ErrorInfo, Params);
		return false;
	}
}
