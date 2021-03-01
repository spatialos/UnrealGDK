#include "Interop/RPCs/SpatialRPCService_2_Senders.h"

namespace SpatialGDK
{
MonotonicRingBufferSender::MonotonicRingBufferSender(ReaderInterface& InReader, WriterInterface& InWriter, int32 NumberOfSlots)
	: Reader(InReader)
	, Writer(InWriter)
	, NumberOfSlots(NumberOfSlots)
{
}

void MonotonicRingBufferSender::OnAuthGained_ReadComponent(const RPCReadingContext& Ctx)
{
	TOptional<uint64> RPCCount = Reader.ReadRPCCount(Ctx);
	BufferState.Add(Ctx.EntityId, RPCCount ? RPCCount.GetValue() : 0);
}

void MonotonicRingBufferSender::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferSender::Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<const FRPCPayload> RPCs, const RPCCallbacks::RPCWritten& WrittenCallback)
{
	uint64& NextSlot = BufferState.FindOrAdd(EntityId, 0);

	uint32_t RPCsToWrite = FMath::Min(RPCs.Num(), NumberOfSlots);
	if (RPCsToWrite > 0)
	{
		auto EntityWrite = Ctx.WriteTo(EntityId, Writer.GetComponentId());
		for (uint32 i = 0; i < RPCsToWrite; ++i)
		{
			uint64 RPCId = ++NextSlot;
			uint64 Slot = (RPCId - 1) % NumberOfSlots;
			Writer.WriteRPC(EntityWrite, Slot, RPCs[i]);
			WrittenCallback(Writer.GetComponentId(), RPCId);
		}
		Writer.WriteRPCCount(EntityWrite, NextSlot);
	}

	return RPCsToWrite;
}

MonotonicRingBufferWithACKSender::MonotonicRingBufferWithACKSender(ReaderInterface& InReader, WriterInterface& InWriter,
																   int32 InNumberOfSlots)
	: Reader(InReader)
	, Writer(InWriter)
	, NumberOfSlots(InNumberOfSlots)
{
}

void MonotonicRingBufferWithACKSender::OnUpdate(const RPCReadingContext& Ctx)
{
	if (Ctx.ComponentId == Reader.GetACKComponentId())
	{
		BufferStateData& State = BufferState.FindChecked(Ctx.EntityId);

		TOptional<uint64> NewACKCount = Reader.ReadACKCount(Ctx);
		if (NewACKCount)
		{
			State.LastACK = NewACKCount.GetValue();
		}
	}
}

void MonotonicRingBufferWithACKSender::OnAuthGained_ReadWriteComponent(const RPCReadingContext& Ctx)
{
	BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);
	TOptional<uint64> RPCCount = Reader.ReadRPCCount(Ctx);
	State.CountWritten = RPCCount ? RPCCount.GetValue() : 0;
}

void MonotonicRingBufferWithACKSender::OnAuthGained_ReadACKComponent(const RPCReadingContext& Ctx)
{
	BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);
	TOptional<uint64> ACKCount = Reader.ReadACKCount(Ctx);
	State.LastACK = ACKCount ? ACKCount.GetValue() : 0;
}

void MonotonicRingBufferWithACKSender::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferWithACKSender::Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<const FRPCPayload> RPCs, const RPCCallbacks::RPCWritten& WrittenCallback)
{
	BufferStateData& NextSlot = BufferState.FindOrAdd(EntityId);
	int32 AvailableSlots = FMath::Max(0, int32(NumberOfSlots) - int32(NextSlot.CountWritten - NextSlot.LastACK));

	int32 RPCsToWrite = FMath::Min(RPCs.Num(), AvailableSlots);
	if (RPCsToWrite > 0)
	{
		auto EntityWrite = Ctx.WriteTo(EntityId, Writer.GetComponentId());
		for (int32 i = 0; i < RPCsToWrite; ++i)
		{
			uint64 RPCId = ++NextSlot.CountWritten;
			uint64 Slot = (RPCId - 1) % NumberOfSlots;
			Writer.WriteRPC(EntityWrite, Slot, RPCs[i]);
			WrittenCallback(Writer.GetComponentId(), RPCId);
		}
		Writer.WriteRPCCount(EntityWrite, NextSlot.CountWritten);
	}

	return RPCsToWrite;
}
} // namespace SpatialGDK
