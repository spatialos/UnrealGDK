// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Containers/Array.h"
#include <improbable/c_worker.h>

#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
// todo - test to see how well code can be de-duplicated. So far it looks like not well.
// Can remove some pretty straight forward boilerplate by introducing both templates and virtual methods.
// Just using static polymorphism might be better but the main problem is needing the change the type of RPCPayloadData.
template <typename RpcMetaData, typename Derived>
class RPCPayloadSender
{
public:
	virtual ~RPCPayloadSender() = default;

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
