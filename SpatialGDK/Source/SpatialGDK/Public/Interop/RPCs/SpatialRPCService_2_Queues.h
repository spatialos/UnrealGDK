// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
template <typename Payload>
struct TRPCLocalOverflowQueue : public TRPCQueue<Payload>
{
	TRPCLocalOverflowQueue(TRPCBufferSender<Payload>& Sender)
		: TRPCQueue(Sender)
	{}
	
	void FlushAll(RPCWritingContext& Ctx) override;
	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx) override;
	void OnAuthLost(Worker_EntityId EntityId) override;
	void OnAuthGained_ReadComponent(RPCReadingContext& iCtx) override;

protected:

	bool FlushQueue(Worker_EntityId EntityId, TArray<Payload>& Queue, RPCWritingContext& Ctx);
};

template <typename Payload>
struct TRPCFixedCapacityQueue : public TRPCLocalOverflowQueue<Payload>
{
	TRPCFixedCapacityQueue(TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: TRPCLocalOverflowQueue(Sender)
		, Capacity(InCapacity)
	{}

	int32 Capacity;

	void Push(Worker_EntityId EntityId, Payload&& Data) override;
	void FlushAll(RPCWritingContext& Ctx) override;
	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx) override;
};

template <typename Payload>
struct TRPCMostRecentQueue : public TRPCFixedCapacityQueue<Payload>
{
	TRPCMostRecentQueue(TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: TRPCFixedCapacityQueue(Sender, InCapacity)
	{}

	void Push(Worker_EntityId EntityId, Payload&& Data) override;
};

template <typename Payload>
void TRPCLocalOverflowQueue<Payload>::FlushAll(RPCWritingContext& Ctx)
{
	for (auto Iterator = Queues.CreateIterator(); Iterator; ++Iterator)
	{
		QueueData& Queue = Iterator->Value;
		if (Queue.bAdded)
		{
			FlushQueue(Iterator->Key, Iterator->Value.RPCs, Ctx);
		}
	}
}

template <typename Payload>
void TRPCLocalOverflowQueue<Payload>::Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx)
{
	QueueData* Queue = Queues.Find(EntityId);
	if (Queue == nullptr || !Queue->bAdded)
	{
		return;
	}

	FlushQueue(EntityId, Queue->RPCs, Ctx);
}

template <typename Payload>
void TRPCLocalOverflowQueue<Payload>::OnAuthGained_ReadComponent(RPCReadingContext& iCtx)
{
}

template <typename Payload>
void TRPCLocalOverflowQueue<Payload>::OnAuthLost(Worker_EntityId EntityId)
{
	Queues.Remove(EntityId);
}

template <typename Payload>
bool TRPCLocalOverflowQueue<Payload>::FlushQueue(Worker_EntityId EntityId, TArray<Payload>& Queue, RPCWritingContext& Ctx)
{
	uint32_t QueuedRPCs = Queue.Num();
	uint32_t WrittenRPCs = Sender.Write(Ctx, EntityId, Queue);

	if (WrittenRPCs == QueuedRPCs)
	{
		Queue.Empty();
		return true;
	}
	else
	{
		for (uint32 i = 0; i < WrittenRPCs; ++i)
		{
			Queue[i] = MoveTemp(Queue[i + WrittenRPCs]);
		}
		Queue.SetNum(QueuedRPCs - WrittenRPCs);
	}

	return false;
}

template <typename Payload>
void TRPCFixedCapacityQueue<Payload>::Push(Worker_EntityId EntityId, Payload&& Data)
{
	QueueData& Queue = Queues.FindOrAdd(EntityId);

	if (Queue.RPCs.Num() < Capacity)
	{
		Queue.RPCs.Add(MoveTemp(Data));
	}
}

template <typename Payload>
void TRPCFixedCapacityQueue<Payload>::FlushAll(RPCWritingContext& Ctx)
{
	for (auto Iterator = Queues.CreateIterator(); Iterator; ++Iterator)
	{
		QueueData& Queue = Iterator->Value;
		if (Queue.bAdded && Queue.RPCs.Num() > 0)
		{
			FlushQueue(Iterator->Key, Queue.RPCs, Ctx);
			Queue.RPCs.Empty();
		}
	}
}

template <typename Payload>
void TRPCFixedCapacityQueue<Payload>::Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx)
{
	QueueData* Queue = Queues.Find(EntityId);
	if (Queue == nullptr || !Queue->bAdded)
	{
		return;
	}
	FlushQueue(EntityId, Queue->RPCs, Ctx);
}

template <typename Payload>
void TRPCMostRecentQueue<Payload>::Push(Worker_EntityId EntityId, Payload&& Data)
{
	QueueData& Queue = Queues.FindOrAdd(EntityId);

	if (Queue.RPCs.Num() == Capacity)
	{
		Queue.RPCs.RemoveAt(0);
	}
	Queue.RPCs.Add(MoveTemp(Data));
}

} // namespace SpatialGDK
