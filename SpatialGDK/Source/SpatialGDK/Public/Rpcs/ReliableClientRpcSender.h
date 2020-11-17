﻿// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Containers/Array.h"
#include <improbable/c_worker.h>

#include "Rpcs/RingBufferWriters.h"
#include "Schema/RPCPayload.h"
#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
class FReliableClientRpcSender
{
public:
	explicit FReliableClientRpcSender(SpatialInterface* Sender, FMonotonicRingBufferWriter RingBufferWriter,
									  FOverflowBufferWriter OverflowWriter);

	void Send(Worker_EntityId EntityId, RPCPayload Rpc);
	void SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc);
	void FlushEntity(Worker_EntityId EntityId);
	void FlushAll();
	void ClearEntity(Worker_EntityId EntityId);

private:
	struct FRpcData
	{
		TArray<RPCPayload> MessagesToSend;
		int32 OverflowSize;
		int64 LastSent;
		int64 LastAck;
	};

	void WriteToEntity(Worker_EntityId EntityId, FRpcData& Data);

	FMonotonicRingBufferWriter RingBufferWriter;
	FOverflowBufferWriter OverflowWriter;
	SpatialInterface* Sender;

	TMap<Worker_EntityId_Key, FRpcData> EntityIdToRpcData;
};
} // namespace SpatialGDK
