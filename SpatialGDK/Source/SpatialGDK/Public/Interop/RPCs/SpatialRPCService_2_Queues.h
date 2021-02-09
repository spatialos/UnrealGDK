// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
struct RPCLocalOverflowQueue : public RPCQueue
{
	void Init(RPCWriter* InWriter) override;
	void Push(Worker_EntityId EntityId, TArray<uint8> Data) override;
	void FlushAll(FlushCallback& CB) override;
	void Flush(Worker_EntityId EntityId, FlushCallback& CB) override;
	void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element) override;
	void OnAuthLost(Worker_EntityId EntityId) override;

protected:
	struct QueuedRPC
	{
		TArray<TArray<uint8>> Data;
	};

	bool FlushQueue(Worker_EntityId EntityId, QueuedRPC& Queue, FlushCallback& CB);
	TMap<Worker_EntityId, QueuedRPC> Queues;
	RPCWriter* Writer;
};

struct RPCFixedCapacityQueue : public RPCLocalOverflowQueue
{
	RPCFixedCapacityQueue(uint32 InCapacity);

	int32 Capacity;

	void Push(Worker_EntityId EntityId, TArray<uint8> Data) override;
	void FlushAll(FlushCallback& CB) override;
	void Flush(Worker_EntityId EntityId, FlushCallback& CB) override;
};

struct RPCMostRecentQueue : public RPCFixedCapacityQueue
{
	RPCMostRecentQueue(uint32 InCapacity);

	void Push(Worker_EntityId EntityId, TArray<uint8> Data) override;
};

} // namespace SpatialGDK
