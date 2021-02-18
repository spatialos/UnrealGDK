#include "Interop/RPCs/SpatialRPCService_2_Receivers.h"

namespace SpatialGDK
{
MonotonicRingBufferWithACKReceiver::MonotonicRingBufferWithACKReceiver(ReaderInterface& InReader, WriterInterface& InWriter,
																	   int32 InNumberOfSlots)
	: Reader(InReader)
	, Writer(InWriter)
	, NumberOfSlots(InNumberOfSlots)
{
}

void MonotonicRingBufferWithACKReceiver::OnAdded(Worker_EntityId EntityId, EntityViewElement const& Element)
{
	const ComponentData* RPCComponentData = nullptr;
	RPCReadingContext readCtx;
	readCtx.EntityId = EntityId;
	for (const auto& Component : Element.Components)
	{
		if (Component.GetComponentId() == Reader.GetComponentId())
		{
			RPCComponentData = &Component;
		}
		else if (ComponentsToRead.Contains(Component.GetComponentId()))
		{
			readCtx.ComponentId = Component.GetComponentId();
			readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

			OnAdded_ReadComponent(readCtx);
		}
	}

	if (RPCComponentData != nullptr)
	{
		readCtx.ComponentId = Reader.GetComponentId();
		readCtx.Fields = Schema_GetComponentDataFields(RPCComponentData->GetUnderlying());

		OnAdded_ReadComponent(readCtx);
	}
}

void MonotonicRingBufferWithACKReceiver::OnAdded_ReadRPCComponent(RPCReadingContext& Ctx)
{
	ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
	ReadRPCs(Ctx, State);
}

void MonotonicRingBufferWithACKReceiver::OnAdded_ReadACKComponent(RPCReadingContext& Ctx)
{
	ReceiverState& State = ReceiverStates.FindOrAdd(Ctx.EntityId);
	State.LastExecuted = State.LastRead = State.LastWrittenACK = Reader.ReadACKCount(Ctx);
}

void MonotonicRingBufferWithACKReceiver::OnRemoved(Worker_EntityId EntityId)
{
	ReceiverStates.Remove(EntityId);
	ReceivedRPCs.Remove(EntityId);
}

void MonotonicRingBufferWithACKReceiver::OnUpdate(RPCReadingContext& Ctx)
{
	if (ComponentsToRead.Contains(Ctx.ComponentId))
	{
		ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
		ReadRPCs(Ctx, State);
	}
}

void MonotonicRingBufferWithACKReceiver::ReadRPCs(RPCReadingContext& Ctx, ReceiverState& State)
{
	uint64 RPCCount = Reader.ReadRPCCount(Ctx);
	if (RPCCount != State.LastRead)
	{
		TArray<RPCPayload>& Received = this->ReceivedRPCs.FindOrAdd(Ctx.EntityId);
		for (uint64 RPCId = State.LastRead + 1; RPCId <= RPCCount; ++RPCId)
		{
			uint32 Slot = (RPCId - 1) % NumberOfSlots;
			RPCPayload NewPayload;
			Reader.ReadRPC(Ctx, Slot, NewPayload);
			Received.Emplace(MoveTemp(NewPayload));
		}

		State.LastRead = RPCCount;
	}
}

void MonotonicRingBufferWithACKReceiver::FlushUpdates(RPCWritingContext& Ctx)
{
	for (auto& Receiver : ReceiverStates)
	{
		Worker_EntityId EntityId = Receiver.Key;
		ReceiverState& State = Receiver.Value;
		if (State.LastExecuted != State.LastWrittenACK)
		{
			auto EntityWriter = Ctx.WriteTo(EntityId, Writer.GetComponentId());
			Writer.WriteACKCount(EntityWriter, State.LastExecuted);
			State.LastWrittenACK = State.LastExecuted;
		}
	}
}

void MonotonicRingBufferWithACKReceiver::ExtractReceivedRPCs(const CanExtractRPCs& CanExtract, const ProcessRPC& Process)
{
	for (auto Iterator = ReceivedRPCs.CreateIterator(); Iterator; ++Iterator)
	{
		Worker_EntityId EntityId = Iterator->Key;
		if (!CanExtract(EntityId))
		{
			continue;
		}
		ReceiverState& State = ReceiverStates.FindChecked(EntityId);
		TArray<RPCPayload>& Payloads = Iterator->Value;
		uint32 ProcessedRPCs = 0;
		for (const auto& Payload : Payloads)
		{
			FUnrealObjectRef Ref(EntityId, Payload.Offset);
			if (Process(Ref, Payload))
			{
				++ProcessedRPCs;
			}
			else
			{
				break;
			}
		}
		if (ProcessedRPCs == Payloads.Num())
		{
			Iterator.RemoveCurrent();
		}
		else
		{
			uint32_t RemainingRPCs = Payloads.Num() - ProcessedRPCs;
			for (uint32_t i = 0; i < RemainingRPCs; ++i)
			{
				Payloads[i] = MoveTemp(Payloads[i + ProcessedRPCs]);
			}
			Payloads.SetNum(RemainingRPCs);
		}

		State.LastExecuted += ProcessedRPCs;
	}
}
} // namespace SpatialGDK
