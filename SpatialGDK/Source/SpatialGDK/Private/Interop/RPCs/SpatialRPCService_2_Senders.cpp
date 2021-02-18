#include "Interop/RPCs/SpatialRPCService_2_Senders.h"

namespace SpatialGDK
{
MonotonicRingBufferSender::MonotonicRingBufferSender(ReaderInterface& InReader, WriterInterface& InWriter, int32 NumberOfSlots)
	: Reader(InReader)
	, Writer(InWriter)
	, NumberOfSlots(NumberOfSlots)
{
}

void MonotonicRingBufferSender::OnAuthGained_ReadComponent(RPCReadingContext& Ctx)
{
	uint64 CounterValue = Reader.ReadRPCCount(Ctx);
	BufferState.Add(Ctx.EntityId, CounterValue);
}

void MonotonicRingBufferSender::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferSender::Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, const TArray<RPCPayload>& RPCs)
{
	uint64& NextSlot = BufferState.FindOrAdd(EntityId, 0);

	uint32_t RPCsToWrite = FMath::Min(RPCs.Num(), NumberOfSlots);
	if (RPCsToWrite > 0)
	{
		auto EntityWrite =  Ctx.WriteTo(EntityId, Writer.GetComponentId());
		for (uint32 i = 0; i < RPCsToWrite; ++i)
		{
			uint64 RPCId = ++NextSlot;
			uint64 Slot = (RPCId - 1) % NumberOfSlots;
			Writer.WriteRPC(EntityWrite, Slot, RPCs[i]);
			EntityWrite.RPCWritten(RPCId);
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

void MonotonicRingBufferWithACKSender::OnUpdate(RPCReadingContext& Ctx)
{
	if (!ComponentsToReadOnUpdate.Contains(Ctx.ComponentId))
	{
		return;
	}

	if (Ctx.ComponentId == Reader.GetACKComponentId())
	{
		BufferStateData& State = BufferState.FindChecked(Ctx.EntityId);

		// !! Reading an update might not have a field !!
		State.LastACK = Reader.ReadACKCount(Ctx);
	}
}

void MonotonicRingBufferWithACKSender::OnAuthGained_ReadWriteComponent(RPCReadingContext& Ctx)
{
	BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);

	State.LastACK = Reader.ReadRPCCount(Ctx);
}

void MonotonicRingBufferWithACKSender::OnAuthGained_ReadACKComponent(RPCReadingContext& Ctx)
{
	BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);

	State.LastACK = Reader.ReadACKCount(Ctx);
}

void MonotonicRingBufferWithACKSender::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferWithACKSender::Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, const TArray<RPCPayload>& RPCs)
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
			EntityWrite.RPCWritten(RPCId);
		}
		Writer.WriteRPCCount(EntityWrite, NextSlot.CountWritten);
	}

	return RPCsToWrite;
}
} // namespace SpatialGDK
