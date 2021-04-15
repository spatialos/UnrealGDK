// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{
// Limit for unbounded queue, where we should consider disconnecting a client.
constexpr int32 GUnboundedQueueOverflowLimit = 1024;

/**
 * Unbounded queue.
 * Will try to flush everything each frame, and keep the Items that could not be flushed for later
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCUnboundedQueue : public TRPCQueue<Payload, AdditionalSendingData>
{
	using Super = TRPCQueue<Payload, AdditionalSendingData>;

	TRPCUnboundedQueue(FName InName, TRPCBufferSender<Payload>& Sender)
		: Super(InName, Sender)
	{
	}

	void FlushAll(RPCWritingContext& Ctx, const typename Super::SentRPCCallback& SentCallback = Super::SentRPCCallback()) override
	{
		for (auto Iterator = this->Queues.CreateIterator(); Iterator; ++Iterator)
		{
			typename Super::QueueData& Queue = Iterator->Value;
			if (Queue.bAdded)
			{
				this->FlushQueue(Iterator->Key, Queue, Ctx, SentCallback);
				CheckOverflow(Iterator->Key, Queue);
			}
		}
	}

	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx,
			   const typename Super::SentRPCCallback& SentCallback = Super::SentRPCCallback(), bool bIgnoreAdded = false) override
	{
		typename Super::QueueData* Queue = this->Queues.Find(EntityId);
		if (Queue == nullptr || (!Queue->bAdded && !bIgnoreAdded))
		{
			return;
		}

		this->FlushQueue(EntityId, *Queue, Ctx, SentCallback);
		CheckOverflow(EntityId, *Queue);
	}

	void OnAuthLost(Worker_EntityId EntityId) override
	{
		Super::OnAuthLost(EntityId);
		OverflowSizes.Remove(EntityId);
	}

protected:
	void CheckOverflow(Worker_EntityId EntityId, typename Super::QueueData& Queue)
	{
		const int32 RemainingRPCs = Queue.RPCs.Num();
		if (RemainingRPCs == 0)
		{
			OverflowSizes.Remove(EntityId);
		}
		else
		{
			int32* Overflow = OverflowSizes.Find(EntityId);
			if (Overflow == nullptr)
			{
				if (this->ErrorCallback)
				{
					this->ErrorCallback(this->Name, EntityId, QueueError::BufferOverflow);
				}
				Overflow = &OverflowSizes.Add(EntityId, RemainingRPCs);
			}
			*Overflow = RemainingRPCs;
			if (*Overflow > GUnboundedQueueOverflowLimit)
			{
				if (this->ErrorCallback)
				{
					this->ErrorCallback(this->Name, EntityId, QueueError::QueueFull);
				}
				// Clear to avoid 1) spamming errors 2) leaking resources.
				Queue.RPCs.Empty();
				Queue.AddData.Empty();
			}
		}
	}

	TMap<Worker_EntityId, int32> OverflowSizes;
};

/**
 * Queue with a fixed capacity.
 * It will queue up to the specified capacity and drop additional RPCs
 * It will drop all Items that were not sent when flushing.
 */
template <typename Payload, typename AdditionalSendingData = RPCEmptyData>
struct TRPCFixedCapacityQueue : public TRPCQueue<Payload, AdditionalSendingData>
{
	using Super = TRPCQueue<Payload, AdditionalSendingData>;

	TRPCFixedCapacityQueue(FName InName, TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: Super(InName, Sender)
		, Capacity(InCapacity)
	{
	}

	int32 Capacity;

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData = AdditionalSendingData()) override
	{
		typename Super::QueueData& Queue = this->Queues.FindOrAdd(EntityId);

		if (Queue.RPCs.Num() < Capacity)
		{
			Queue.RPCs.Add(MoveTemp(Data));
			Queue.AddData.Add(AddData);
		}
		else if (this->ErrorCallback)
		{
			this->ErrorCallback(this->Name, EntityId, QueueError::QueueFull);
		}
	}

	void FlushAll(RPCWritingContext& Ctx, const typename Super::SentRPCCallback& SentCallback = Super::SentRPCCallback()) override
	{
		for (auto Iterator = this->Queues.CreateIterator(); Iterator; ++Iterator)
		{
			typename Super::QueueData& Queue = Iterator->Value;
			if (Queue.bAdded && Queue.RPCs.Num() > 0)
			{
				this->FlushQueue(Iterator->Key, Queue, Ctx, SentCallback);
				Queue.RPCs.Empty();
				Queue.AddData.Empty();
			}
		}
	}

	void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx,
			   const typename Super::SentRPCCallback& SentCallback = Super::SentRPCCallback(), bool bIgnoreAdded = false) override
	{
		typename Super::QueueData* Queue = this->Queues.Find(EntityId);
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
	using Super = TRPCFixedCapacityQueue<Payload, AdditionalSendingData>;

	TRPCMostRecentQueue(FName InName, TRPCBufferSender<Payload>& Sender, uint32 InCapacity)
		: Super(InName, Sender, InCapacity)
	{
	}

	void Push(Worker_EntityId EntityId, Payload&& Data, AdditionalSendingData&& AddData = AdditionalSendingData()) override
	{
		typename Super::QueueData& Queue = this->Queues.FindOrAdd(EntityId);

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
