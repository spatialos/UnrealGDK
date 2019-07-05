// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/RPCContainer.h"

using namespace SpatialGDK;

FPendingRPCParams::FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, RPCPayload&& InPayload, int InReliableRPCIndex /*= 0*/)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, ReliableRPCIndex(InReliableRPCIndex)
	, Payload(MoveTemp(InPayload))
{
}

void FRPCContainer::QueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type)
{
	if (!Params->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}
	UObject* TargetObject = Params->TargetObject.Get();

	QueueRPC(TargetObject, Type, Params);
}

void FRPCContainer::QueueRPC(const UObject* TargetObject, ESchemaComponentType Type, FPendingRPCParamsPtr Params)
{
	check(TargetObject);
	FArrayOfParams& ArrayOfParams = QueuedRPCs.FindOrAdd(Type).FindOrAdd(TargetObject);
	ArrayOfParams.Push(Params);
}

void FRPCContainer::ProcessRPCs(const FProcessRPCDelegate& FunctionToApply, FArrayOfParams& RPCList)
{
	int NumProcessedParams = 0;
	for (auto& Params : RPCList)
	{
		if (!Params->TargetObject.IsValid())
		{
			// The target object was destroyed before we could send the RPC.
			RPCList.Empty();
			break;
		}
		if (ApplyFunction(FunctionToApply, Params))
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

bool FRPCContainer::ObjectHasRPCsQueuedOfType(const UObject* TargetObject, ESchemaComponentType Type) const
{
	const FRPCMap* MapOfQueues = QueuedRPCs.Find(Type);
	if(MapOfQueues)
	{
		const FArrayOfParams* RPCList = MapOfQueues->Find(TargetObject);
		if(RPCList)
		{
			return (RPCList->Num() > 0);
		}
	}

	return false;
}

bool FRPCContainer::ApplyFunction(const FProcessRPCDelegate& FunctionToApply, FPendingRPCParamsPtr Params)
{
	return FunctionToApply.Execute(Params);
}
