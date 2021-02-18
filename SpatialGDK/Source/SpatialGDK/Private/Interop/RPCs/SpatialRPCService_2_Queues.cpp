#include "Interop/RPCs/SpatialRPCService_2_Queues.h"

#if 0
namespace SpatialGDK
{
void RPCLocalOverflowQueue::Init(RPCWriter* InWriter)
{
	Writer = InWriter;
}

void RPCLocalOverflowQueue::Push(Worker_EntityId EntityId, TArray<uint8> Data)
{
	QueuedRPC& Queue = Queues.FindOrAdd(EntityId, QueuedRPC());
	// if (Queue == nullptr)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Creating queue for entity %llu"), EntityId);
	//	Queue = &Queues.Add(EntityId, QueuedRPC());
	//}

	Queue.Data.Add(MoveTemp(Data));
}

void RPCLocalOverflowQueue::FlushAll(FlushCallback& CB)
{
	for (auto Iterator = Queues.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->Value.Data.Num() == 0 || FlushQueue(Iterator->Key, Iterator->Value, CB))
		{
			Iterator.RemoveCurrent();
		}
	}
}

void RPCLocalOverflowQueue::Flush(Worker_EntityId EntityId, FlushCallback& CB)
{
	QueuedRPC* Queue = Queues.Find(EntityId);
	if (Queue == nullptr)
	{
		return;
	}
	if (FlushQueue(EntityId, *Queue, CB))
	{
		Queues.Remove(EntityId);
	}
}

void RPCLocalOverflowQueue::OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element)
{
	// Queues.Add(EntityId, QueuedRPC());
}

void RPCLocalOverflowQueue::OnAuthLost(Worker_EntityId EntityId)
{
	Queues.Remove(EntityId);
}

bool RPCLocalOverflowQueue::FlushQueue(Worker_EntityId EntityId, QueuedRPC& Queue, FlushCallback& CB)
{
	uint32_t QueuedRPCs = Queue.Data.Num();
	uint32_t AvailableSpace = Writer->GetFreeSlots(EntityId);
	uint32_t RPCsToWrite = FMath::Min(QueuedRPCs, AvailableSpace);
	if (RPCsToWrite)
	{
		CB(EntityId, Queue.Data.GetData(), RPCsToWrite);
		if (RPCsToWrite == QueuedRPCs)
		{
			Queue.Data.Empty();
			return true;
		}
		else
		{
			for (uint32 i = 0; i < RPCsToWrite; ++i)
			{
				Queue.Data[i] = MoveTemp(Queue.Data[i + RPCsToWrite]);
			}
			Queue.Data.SetNum(QueuedRPCs - RPCsToWrite);
		}
	}

	return false;
}

RPCFixedCapacityQueue::RPCFixedCapacityQueue(uint32 InCapacity)
	: Capacity(InCapacity)
{
}

void RPCFixedCapacityQueue::Push(Worker_EntityId EntityId, TArray<uint8> Data)
{
	QueuedRPC& Queue = Queues.FindOrAdd(EntityId, QueuedRPC());

	if (Queue.Data.Num() < Capacity)
	{
		Queue.Data.Add(MoveTemp(Data));
	}
}

void RPCFixedCapacityQueue::FlushAll(FlushCallback& CB)
{
	for (auto Iterator = Queues.CreateIterator(); Iterator; ++Iterator)
	{
		if (Iterator->Value.Data.Num() == 0)
		{
			FlushQueue(Iterator->Key, Iterator->Value, CB);
			Iterator.RemoveCurrent();
		}
	}
}

void RPCFixedCapacityQueue::Flush(Worker_EntityId EntityId, FlushCallback& CB)
{
	QueuedRPC* Queue = Queues.Find(EntityId);
	if (Queue == nullptr)
	{
		return;
	}
	FlushQueue(EntityId, *Queue, CB);
	Queues.Remove(EntityId);
}

RPCMostRecentQueue::RPCMostRecentQueue(uint32 InCapacity)
	: RPCFixedCapacityQueue(InCapacity)
{
}

void RPCMostRecentQueue::Push(Worker_EntityId EntityId, TArray<uint8> Data)
{
	QueuedRPC& Queue = Queues.FindOrAdd(EntityId, QueuedRPC());

	if (Queue.Data.Num() == Capacity)
	{
		Queue.Data.RemoveAt(0);
	}
	Queue.Data.Add(MoveTemp(Data));
}

} // namespace SpatialGDK
#endif
