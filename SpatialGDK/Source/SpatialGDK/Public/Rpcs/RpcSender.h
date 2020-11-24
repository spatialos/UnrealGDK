// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Containers/Array.h"
#include <improbable/c_worker.h>

#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
// todo - this is a test to see how well code can be de-duplicated. So far it looks like not well.
// Can remove some pretty straight forward boilerplate by introducing both templates and inheritance.
// Just using static polymorphism might be better than making things virtual but the main problem is needing the change the
// type of RPCPayloadData as well as needing different information in each constructor which means none of them end up being all that clean.
template <typename RpcMetaData, typename Derived>
class PossibleRPCPayloadSender
{
public:
	virtual ~PossibleRPCPayloadSender() = default;

	void Send(Worker_EntityId EntityId, RPCPayload Rpc);
	void SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc);
	void FlushEntity(Worker_EntityId EntityId);
	void FlushAll();
	void ClearEntity(Worker_EntityId EntityId);

private:
	struct RPCPayloadData
	{
		TArray<RPCPayload> MessagesToSend;
		RpcMetaData Data;
	};

	virtual void WriteToEntity(Worker_EntityId EntityId, RPCPayloadData& Data) = 0;

	TMap<Worker_EntityId_Key, RPCPayloadData> EntityIdToRpcData;
};
} // namespace SpatialGDK
