// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCContainer.h"

#include "Schema/UnrealObjectRef.h"

using namespace SpatialGDK;

FPendingRPCParams::FPendingRPCParams(const FUnrealObjectRef& InTargetObjectRef, SpatialGDK::RPCPayload&& InPayload, int InReliableRPCIndex /* = 0 */)
	: ReliableRPCIndex(InReliableRPCIndex)
	, ObjectRef(InTargetObjectRef)
	, Payload(MoveTemp(InPayload))
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
	return FunctionToApply.Execute(Params);
}
