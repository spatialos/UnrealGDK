#pragma once

#include "SpatialCommonTypes.h"
#include "Utils/RPCRingBuffer.h"

#include "Containers/BitArray.h"

namespace SpatialGDK
{
namespace CrossServer
{
enum class Result
{
	Success,
	TargetDestroyed,
	TargetUnknown,
};

inline void WritePayloadAndCounterpart(Schema_Object* EndpointObject, const RPCPayload& Payload, const CrossServerRPCInfo& Info,
									   uint32_t SlotIdx)
{
	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::CrossServer);
	uint32 Field = Descriptor.GetRingBufferElementFieldId(ERPCType::CrossServer, SlotIdx + 1);

	Schema_Object* RPCObject = Schema_AddObject(EndpointObject, Field);
	Payload.WriteToSchemaObject(RPCObject);

	Info.AddToSchema(EndpointObject, Field + 1);
}

typedef TPair<Worker_EntityId_Key, uint64> RPCKey;

struct SentRPCEntry
{
	bool operator==(SentRPCEntry const& iRHS) const { return FMemory::Memcmp(this, &iRHS, sizeof(SentRPCEntry)) == 0; }

	RPCTarget Target;
	uint32 SourceSlot;
	TOptional<uint32> DestinationSlot;
};

// Helper to remember which slots are occupied and which slots should be cleared for the next update.
struct SlotAlloc
{
	SlotAlloc()
	{
		Occupied.Init(false, RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer));
		ToClear.Init(false, RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer));
	}

	TOptional<uint32_t> PeekFreeSlot()
	{
		int32 freeSlot = Occupied.Find(false);
		if (freeSlot >= 0)
		{
			return freeSlot;
		}

		return {};
	}

	void CommitSlot(uint32_t Slot)
	{
		Occupied[Slot] = true;
		ToClear[Slot] = false;
	}

	TOptional<uint32_t> ReserveSlot()
	{
		int32 freeSlot = Occupied.FindAndSetFirstZeroBit();
		if (freeSlot >= 0)
		{
			CommitSlot(freeSlot);
			return freeSlot;
		}

		return {};
	}

	void FreeSlot(uint32_t Slot)
	{
		Occupied[Slot] = false;
		ToClear[Slot] = true;
	}

	template <typename Functor>
	void ForeachClearedSlot(Functor&& Fun)
	{
		for (int32 ToClearIdx = ToClear.Find(true); ToClearIdx >= 0; ToClearIdx = ToClear.Find(true))
		{
			Fun(ToClearIdx);
			ToClear[ToClearIdx] = false;
		}
	}

	TBitArray<FDefaultBitArrayAllocator> Occupied;
	TBitArray<FDefaultBitArrayAllocator> ToClear;
};

// Helper to order arrival of RPCs to a receiver
// Enforces ordering when RPCs are from the same sender.
// Otherwise, keep arrival ordering to prevent starvation.
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

struct WriterState
{
	uint64 LastSentRPCId = 0;
	TMap<RPCKey, SentRPCEntry> Mailbox;
	SlotAlloc Alloc;
};

struct RPCSlots
{
	Worker_EntityId CounterpartEntity;
	int32 CounterpartSlot = -1;
	int32 ACKSlot = -1;
};

using ReadRPCMap = TMap<CrossServer::RPCKey, RPCSlots>;

struct ReaderState
{
	ReadRPCMap RPCSlots;
	SlotAlloc ACKAlloc;
};

} // namespace CrossServer
} // namespace SpatialGDK
