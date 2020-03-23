// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "ConnectionHandlers/AbstractConnectionHandler.h"
#include "OpList/AbstractOpList.h"
#include "Containers/Array.h"

namespace SpatialGDK
{

class QueuedOpListConnectionHandler : public AbstractConnectionHandler
{
public:
	explicit QueuedOpListConnectionHandler(Worker_Connection* Connection)
	: Connection(Connection)
	{
	}

	void Advance() override
	{
	}

	uint32 GetOpListCount() override
	{
		return OpLists.Num();
	}

	TUniquePtr<AbstractOpList> GetNextOpList() override
	{
		TUniquePtr<AbstractOpList> NextOpList = MoveTemp(OpLists[0]);
		OpLists.RemoveAt(0);
		return NextOpList;
	}

	void EnqueueOpList(TUniquePtr<AbstractOpList> OpList)
	{
		OpLists.Push(MoveTemp(OpList));
	}

	void SendMessages(TUniquePtr<MessagesToSend> Messages) override
	{
		for (auto& Request : Messages->CreateEntityRequests)
		{
			Worker_EntityId* EntityId = Request.EntityId.IsSet() ? &Request.EntityId.GetValue() : nullptr;
			uint32* TimeoutMillis = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
			Worker_Connection_SendCreateEntityRequest(Connection, Request.ComponentCount, Request.EntityComponents, EntityId, TimeoutMillis);
		}
	}

private:
	Worker_Connection* Connection;
	TArray<TUniquePtr<AbstractOpList>> OpLists;
};

}  // namespace SpatialGDK
