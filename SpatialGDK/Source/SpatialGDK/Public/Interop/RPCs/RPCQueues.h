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

	void FlushAll(RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback) override
	{
		for (auto Iterator = this->Queues.CreateIterator(); Iterator; ++Iterator)
		{
			typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = Iterator->Value;
			if (Queue.bAdded)
			{
				this->FlushQueue(Iterator->Key, Queue, Ctx, SentCallback);
			}
		}
	}

	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx,
			   const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback, bool bIgnoreAdded = false) override
	{
		typename TRPCQueue<Payload, AdditionalSendingData>::QueueData* Queue = this->Queues.Find(EntityId);
		if (Queue == nullptr || (!Queue->bAdded && !bIgnoreAdded))
		{
			return;
		}

		this->FlushQueue(EntityId, *Queue, Ctx, SentCallback);
	}
};

/**
 * Queue with a fixed capacity.
 * It will queue up to the specified capacity and drop additional RPCs
 * It will drop all Items that were not sent when flushing.
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCFixedCapacityQueue : public TRPCQueue<Payload, AdditionalSendingData>
{
	TRPCFixedCapacityQueue(FName InName, TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: TRPCUnboundedQueue<Payload, AdditionalSendingData>(InName, Sender)
		, Capacity(InCapacity)
	{
	}

	int32 Capacity;

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData) override
	{
		typename TRPCQueue<Payload, AdditionalSendingData>::QueueData& Queue = this->Queues.FindOrAdd(EntityId);

		if (Queue.RPCs.Num() < Capacity)
		{
			Queue.RPCs.Add(MoveTemp(Data));
			Queue.AddData.Add(AddData);
		}
	}

	void FlushAll(RPCWritingContext& Ctx, const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback) override
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

	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx,
			   const typename TWrappedRPCQueue<AdditionalSendingData>::SentRPCCallback& SentCallback, bool bIgnoreAdded = false) override
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

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData) override
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
};

} // namespace SpatialGDK
