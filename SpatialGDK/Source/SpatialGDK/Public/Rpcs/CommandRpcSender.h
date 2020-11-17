// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Containers/Array.h"
#include <improbable/c_worker.h>

#include "Rpcs/CommandRpcWriter.h"
#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
class FCommandRpcSender
{
public:
	explicit FCommandRpcSender(SpatialInterface* Sender, FCommandRpcWriter RpcWriter);

	void SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc);

private:
	struct FRpcData
	{
		TArray<RPCPayload> MessagesToSend;
	};

	FCommandRpcWriter RpcWriter;
	SpatialInterface* Sender;
	TMap<Worker_EntityId_Key, FRpcData> EntityIdToRpcData;
};
} // namespace SpatialGDK
