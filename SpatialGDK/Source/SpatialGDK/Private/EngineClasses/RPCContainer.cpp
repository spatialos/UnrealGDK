// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RPCContainer.h"

namespace SpatialGDK
{
FPendingRPCParams::FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, int InReliableRPCIndex /* = 0 */)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, ReliableRPCIndex(InReliableRPCIndex)
	, Payload(0, 0, TArray<uint8>{})
{
}

void RPCContainer::QueueRPC(FPendingRPCParamsPtr Params, ESchemaComponentType Type)
{
	if (!Params->TargetObject.IsValid())
	{
		// Target object was destroyed before the RPC could be (re)sent
		return;
	}
	UObject* TargetObject = Params->TargetObject.Get();

	QueueRPC(TargetObject, Type, Params);
}

void RPCContainer::QueueRPC(const UObject* TargetObject, ESchemaComponentType Type, FPendingRPCParamsPtr Params)
{
	check(TargetObject);
	TSharedPtr<FQueueOfParams>& QueuePtr = QueuedRPCs.FindOrAdd(Type).FindOrAdd(TargetObject);
	if (!QueuePtr.IsValid())
	{
		QueuePtr = MakeShared<FQueueOfParams>();
	}
	QueuePtr->Enqueue(Params);
}

void RPCContainer::ProcessRPCs(const FProcessRPCDelegate& FunctionToApply, FQueueOfParams* RPCList)
{
	check(RPCList);
	FPendingRPCParamsPtr RPCParams = nullptr;
	while (!RPCList->IsEmpty())
	{
		RPCList->Peek(RPCParams);
		if (!RPCParams.IsValid())
		{
			//UE_LOG(LogSpatialSender, Warning, TEXT("===RPC Param Invalid"));
			RPCList->Empty();
			break;
		}

		if (!RPCParams->TargetObject.IsValid())
		{
			// The target object was destroyed before we could send the RPC.
			RPCList->Empty();
			break;
		}

		if (ApplyFunction(FunctionToApply, RPCParams))
		{
			RPCList->Pop();
		}
		else
		{
			break;
		}
	}
}

void RPCContainer::ProcessRPCs(const FProcessRPCDelegate& FunctionToApply)
{
	for (auto& RPCs : QueuedRPCs)
	{
		for (auto& RPCMapItem : RPCs.Value)
		{
			TSharedPtr<FQueueOfParams> RPCList = RPCMapItem.Value;
			ProcessRPCs(FunctionToApply, RPCList.Get());
			if ((*RPCList).IsEmpty())
			{
				// TO-DO: add proper cleanup
				//RPCs.Value.Remove(RPCMapItem.Key);
			}
		}
	}
}

bool RPCContainer::ApplyFunction(const FProcessRPCDelegate& FunctionToApply, FPendingRPCParamsPtr Params)
{
	return FunctionToApply.Execute(Params);
}
}
