

#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/RPCRingBuffer.h"

#include "Containers/BitArray.h"

namespace SpatialGDK
{
namespace CrossServer
{
typedef TPair<Worker_EntityId_Key, uint64> RPCKey;

struct SentRPCEntry
{
	bool operator==(SentRPCEntry const& iRHS) const { return FMemory::Memcmp(this, &iRHS, sizeof(SentRPCEntry)) == 0; }

	RPCKey RPCId;
	Worker_EntityId Target;
	uint64 Timestamp;
	uint32 SourceSlot;
	TOptional<uint32> DestinationSlot;
	TOptional<Worker_RequestId> EntityRequest;
};

struct SlotAlloc
{
	SlotAlloc()
	{
		Occupied.Init(false, RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServerSender));
		ToClear.Init(false, RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServerSender));
	}

	TBitArray<FDefaultBitArrayAllocator> Occupied;
	TBitArray<FDefaultBitArrayAllocator> ToClear;
};

struct RPCSchedule
{
	void Add(RPCKey RPC)
	{
		int32 ScheduleSize = SendingSchedule.Num();
		SendingSchedule.Add(RPC);
		RPCKey& LastAddition = SendingSchedule[ScheduleSize];
		for (int32 i = 0; i < ScheduleSize; ++i)
		{
			RPCKey& Item = SendingSchedule[i];
			if (Item.Get<0>() == LastAddition.Get<0>())
			{
				// Enforce ordering on same senders while keeping different senders order of arrival to avoid starvation.
				if (Item.Get<1>() > LastAddition.Get<1>())
				{
					Swap(Item, LastAddition);
				}
			}
		}
	}

	RPCKey Peek() { return SendingSchedule[0]; }

	RPCKey Extract()
	{
		RPCKey NextRPC = SendingSchedule[0];
		SendingSchedule.RemoveAt(0);
		return NextRPC;
	}

	bool IsEmpty() const { return SendingSchedule.Num() == 0; }

	TArray<RPCKey> SendingSchedule;
};

struct SenderState
{
	TOptional<uint32_t> FindFreeSlot()
	{
		int32 freeSlot = Alloc.Occupied.Find(false);
		if (freeSlot >= 0)
		{
			return freeSlot;
		}

		return {};
	}

	TMap<RPCKey, SentRPCEntry> Mailbox;
	RPCSchedule Schedule;

	bool CompareRPC(RPCKey const& iKey1, RPCKey const& iKey2)
	{
		if (iKey1.Get<0>() == iKey2.Get<0>())
		{
			return iKey1.Get<1>() < iKey2.Get<1>();
		}

		SentRPCEntry& RPC1 = Mailbox.FindChecked(iKey1);
		SentRPCEntry& RPC2 = Mailbox.FindChecked(iKey2);

		return RPC1.Timestamp < RPC2.Timestamp;
	}

	SlotAlloc Alloc;
};

struct ACKSlot
{
	bool operator==(const ACKSlot& Other) const { return Receiver == Other.Receiver && Slot == Other.Slot; }

	Worker_EntityId Receiver = 0;
	uint32 Slot = -1;
};

struct RPCSlots
{
	ACKSlot ReceiverSlot;
	int32 SenderACKSlot = -1;
};

typedef TMap<RPCKey, RPCSlots> RPCAllocMap;
} // namespace CrossServer
} // namespace SpatialGDK
