// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{

/**
 * Unbounded queue.
 * Will try to flush everything each frame, and keep the Items that could not be flushed for later
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCUnboundedQueue : public TRPCQueue<Payload, AdditionalSendingData>
{
	TRPCUnboundedQueue(FName InName, TRPCBufferSender<Payload>& Sender)
		: TRPCQueue<Payload, AdditionalSendingData>(InName, Sender)
	{
	}

	void FlushAll(RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback&) override;
	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback&,
			   bool bIgnoreAdded = false) override;
	void OnAuthLost(Worker_EntityId EntityId) override;
	void OnAuthGained_ReadComponent(const RPCReadingContext& iCtx) override;

protected:
	bool FlushQueue(Worker_EntityId EntityId, typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue, RPCWritingContext& Ctx,
					const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback&);
};

/**
 * Queue with a fixed capacity.
 * It will queue up to the specified capacity and drop additional RPCs
 * It will drop all Items that were not sent when flushing.
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCFixedCapacityQueue : public TRPCUnboundedQueue<Payload, AdditionalSendingData>
{
	TRPCFixedCapacityQueue(FName InName, TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: TRPCUnboundedQueue<Payload, AdditionalSendingData>(InName, Sender)
		, Capacity(InCapacity)
	{
	}

	int32 Capacity;

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData) override;
	void FlushAll(RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback&) override;
	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback&,
			   bool bIgnoreAdded = false) override;
};

/**
 * Queue with a fixed capacity that will replace older elements with newer.
 * Behaves like the fixed capacity queue otherwise.
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCMostRecentQueue : public TRPCFixedCapacityQueue<Payload, AdditionalSendingData>
{
	TRPCMostRecentQueue(FName InName, TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: TRPCFixedCapacityQueue<Payload, AdditionalSendingData>(InName, Sender, InCapacity)
	{
	}

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData) override;
};

template <typename Payload, typename AdditionalSendingData>
void TRPCUnboundedQueue<Payload, AdditionalSendingData>::FlushAll(
	RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback)
{
	for (auto Iterator = this->Queues.CreateIterator(); Iterator; ++Iterator)
	{
		typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = Iterator->Value;
		if (Queue.bAdded)
		{
			FlushQueue(Iterator->Key, Queue, Ctx, SentCallback);
		}
	}
}

template <typename Payload, typename AdditionalSendingData>
void TRPCUnboundedQueue<Payload, AdditionalSendingData>::Flush(
	Worker_EntityId EntityId, RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback,
	bool bIgnoreAdded)
{
	typename TRPCQueue<Payload, AdditionalSendingData>::QueueData* Queue = this->Queues.Find(EntityId);
	if (Queue == nullptr || (!Queue->bAdded && !bIgnoreAdded))
	{
		return;
	}

	FlushQueue(EntityId, *Queue, Ctx, SentCallback);
}

template <typename Payload, typename AdditionalSendingData>
void TRPCUnboundedQueue<Payload, AdditionalSendingData>::OnAuthGained_ReadComponent(const RPCReadingContext& iCtx)
{
}

template <typename Payload, typename AdditionalSendingData>
void TRPCUnboundedQueue<Payload, AdditionalSendingData>::OnAuthLost(Worker_EntityId EntityId)
{
	this->Queues.Remove(EntityId);
}

template <typename Payload, typename AdditionalSendingData>
bool TRPCUnboundedQueue<Payload, AdditionalSendingData>::FlushQueue(
	Worker_EntityId EntityId, typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue, RPCWritingContext& Ctx,
	const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback)
{
	uint32 QueuedRPCs = Queue.RPCs.Num();
	uint32 WrittenRPCs = 0;
	auto WrittenCallback = [this, &Queue, EntityId, &SentCallback, &WrittenRPCs](Worker_ComponentId ComponentId, uint64 RPCId) {
		if (SentCallback)
		{
			SentCallback(this->Name, EntityId, ComponentId, RPCId, Queue.AddData[WrittenRPCs]);
		}
		++WrittenRPCs;
	};

	const uint32 WrittenRPCsReported = this->Sender.Write(Ctx, EntityId, Queue.RPCs, WrittenCallback);

	// Basic check that the written callback was called for every individual RPC.
	check(WrittenRPCs == WrittenRPCsReported);

	if (WrittenRPCs == QueuedRPCs)
	{
		Queue.RPCs.Empty();
		Queue.AddData.Empty();
		return true;
	}
	else
	{
		for (uint32 i = 0; i < WrittenRPCs; ++i)
		{
			Queue.RPCs[i] = MoveTemp(Queue.RPCs[i + WrittenRPCs]);
			Queue.AddData[i] = MoveTemp(Queue.AddData[i + WrittenRPCs]);
		}
		Queue.RPCs.SetNum(QueuedRPCs - WrittenRPCs);
		Queue.AddData.SetNum(QueuedRPCs - WrittenRPCs);
	}

	return false;
}

template <typename Payload, typename AdditionalSendingData>
void TRPCFixedCapacityQueue<Payload, AdditionalSendingData>::Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData)
{
	typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = this->Queues.FindOrAdd(EntityId);

	if (Queue.RPCs.Num() < Capacity)
	{
		Queue.RPCs.Add(MoveTemp(Data));
		Queue.AddData.Add(AddData);
	}
}

template <typename Payload, typename AdditionalSendingData>
void TRPCFixedCapacityQueue<Payload, AdditionalSendingData>::FlushAll(
	RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback)
{
	for (auto Iterator = this->Queues.CreateIterator(); Iterator; ++Iterator)
	{
		typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = Iterator->Value;
		if (Queue.bAdded && Queue.RPCs.Num() > 0)
		{
			this->FlushQueue(Iterator->Key, Queue, Ctx, SentCallback);
			Queue.RPCs.Empty();
			Queue.AddData.Empty();
		}
	}
}

template <typename Payload, typename AdditionalSendingData>
void TRPCFixedCapacityQueue<Payload, AdditionalSendingData>::Flush(
	Worker_EntityId EntityId, RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback,
	bool bIgnoreAdded)
{
	typename TRPCQueue<Payload, AdditionalSendingData>::QueueData* Queue = this->Queues.Find(EntityId);
	if (Queue == nullptr || (!Queue->bAdded && !bIgnoreAdded))
	{
		return;
	}
	this->FlushQueue(EntityId, *Queue, Ctx, SentCallback);
	Queue->RPCs.Empty();
	Queue->AddData.Empty();
}

template <typename Payload, typename AdditionalSendingData>
void TRPCMostRecentQueue<Payload, AdditionalSendingData>::Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData)
{
	typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = this->Queues.FindOrAdd(EntityId);

	if (Queue.RPCs.Num() == this->Capacity)
	{
		Queue.RPCs.RemoveAt(0);
		Queue.AddData.RemoveAt(0);
	}
	Queue.RPCs.Add(MoveTemp(Data));
	Queue.AddData.Add(MoveTemp(AddData));
}

} // namespace SpatialGDK
