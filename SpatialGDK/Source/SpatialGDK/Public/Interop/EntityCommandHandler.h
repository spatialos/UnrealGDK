// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCommandRequestWithOp, const Worker_Op&, const Worker_CommandRequestOp&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCommandResponseWithOp, const Worker_Op&, const Worker_CommandResponseOp&);

class EntityCommandRequestHandler
{
public:
	FDelegateHandle AddRequestHandler(const Worker_ComponentId ComponentId, const Worker_CommandIndex CommandIndex,
									  FOnCommandRequestWithOp::FDelegate&& Handler)
	{
		return RequestHandlers.FindOrAdd({ ComponentId, CommandIndex }).Add(Handler);
	}

	void ProcessOps(const TArray<Worker_Op>& Ops) const
	{
		for (const Worker_Op& Op : Ops)
		{
			HandleOp(Op);
		}
	}

	void HandleOp(const Worker_Op& Op) const
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST)
		{
			const Worker_CommandRequestOp& CommandRequestOp = Op.op.command_request;
			const FOnCommandRequestWithOp* RequestHandlerPtr =
				RequestHandlers.Find({ CommandRequestOp.request.component_id, CommandRequestOp.request.command_index });
			if (RequestHandlerPtr != nullptr)
			{
				RequestHandlerPtr->Broadcast(Op, CommandRequestOp);
			}
		}
	}

private:
	struct CommandRequestKey
	{
		Worker_ComponentId ComponentId;
		Worker_CommandIndex CommandIndex;

		friend uint32 GetTypeHash(const CommandRequestKey& Value)
		{
			return HashCombine(::GetTypeHash(Value.ComponentId), ::GetTypeHash(Value.CommandIndex));
		}

		friend bool operator==(const CommandRequestKey& Lhs, const CommandRequestKey& Rhs)
		{
			return Lhs.ComponentId == Rhs.ComponentId && Lhs.CommandIndex == Rhs.CommandIndex;
		}
	};

	TMap<CommandRequestKey, FOnCommandRequestWithOp> RequestHandlers;
	FOnCommandRequestWithOp RequestWithOpHandler;
};

class EntityCommandResponseHandler
{
public:
	FDelegateHandle AddResponseHandler(const Worker_ComponentId ComponentId, const Worker_CommandIndex CommandIndex,
									   FOnCommandResponseWithOp::FDelegate&& Handler)
	{
		return RequestHandlers.FindOrAdd({ ComponentId, CommandIndex }).Add(Handler);
	}

	void ProcessOps(const TArray<Worker_Op>& Ops) const
	{
		for (const Worker_Op& Op : Ops)
		{
			HandleOp(Op);
		}
	}

	void HandleOp(const Worker_Op& Op) const
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST)
		{
			const Worker_CommandResponseOp& CommandRequestOp = Op.op.command_response;
			const FOnCommandResponseWithOp* RequestHandlerPtr =
				RequestHandlers.Find({ CommandRequestOp.response.component_id, CommandRequestOp.response.command_index });
			if (RequestHandlerPtr != nullptr)
			{
				RequestHandlerPtr->Broadcast(Op, CommandRequestOp);
			}
		}
	}

private:
	struct CommandRequestKey
	{
		Worker_ComponentId ComponentId;
		Worker_CommandIndex CommandIndex;

		friend uint32 GetTypeHash(const CommandRequestKey& Value)
		{
			return HashCombine(::GetTypeHash(Value.ComponentId), ::GetTypeHash(Value.CommandIndex));
		}

		friend bool operator==(const CommandRequestKey& Lhs, const CommandRequestKey& Rhs)
		{
			return Lhs.ComponentId == Rhs.ComponentId && Lhs.CommandIndex == Rhs.CommandIndex;
		}
	};

	TMap<CommandRequestKey, FOnCommandResponseWithOp> RequestHandlers;
};

} // namespace SpatialGDK
