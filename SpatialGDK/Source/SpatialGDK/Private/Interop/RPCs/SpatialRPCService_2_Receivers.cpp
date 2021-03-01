#include "Interop/RPCs/SpatialRPCService_2_Receivers.h"

namespace SpatialGDK
{
MonotonicRingBufferWithACKReceiver::MonotonicRingBufferWithACKReceiver(TimestampAndETWrapper<FRPCPayload>&& InWrapper,
	ReaderInterface& InReader, WriterInterface& InWriter,
	int32 InNumberOfSlots)
	: TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>(MoveTemp(InWrapper))
	, Reader(InReader)
	, Writer(InWriter)
	, NumberOfSlots(InNumberOfSlots)
{
}

void MonotonicRingBufferWithACKReceiver::OnAdded(FName ReceiverName, Worker_EntityId EntityId, EntityViewElement const& Element)
{
	const ComponentData* RPCComponentData = nullptr;
	RPCReadingContext readCtx;
	readCtx.ReaderName = ReceiverName;
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

void MonotonicRingBufferWithACKReceiver::OnAdded_ReadRPCComponent(const RPCReadingContext& Ctx)
{
	ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
	ReadRPCs(Ctx, State);
}

void MonotonicRingBufferWithACKReceiver::OnAdded_ReadACKComponent(const RPCReadingContext& Ctx)
{
	ReceiverState& State = ReceiverStates.FindOrAdd(Ctx.EntityId);
	TOptional<uint64> ACKCount = Reader.ReadACKCount(Ctx);
	State.LastExecuted = State.LastRead = State.LastWrittenACK = ACKCount ? ACKCount.GetValue() : 0;
}

void MonotonicRingBufferWithACKReceiver::OnRemoved(Worker_EntityId EntityId)
{
	ReceiverStates.Remove(EntityId);
	ReceivedRPCs.Remove(EntityId);
}

void MonotonicRingBufferWithACKReceiver::OnUpdate(const RPCReadingContext& Ctx)
{
	if (ComponentsToRead.Contains(Ctx.ComponentId))
	{
		ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
		ReadRPCs(Ctx, State);
	}
}

void MonotonicRingBufferWithACKReceiver::ReadRPCs(const RPCReadingContext& Ctx, ReceiverState& State)
{
	TOptional<uint64> NewRPCCount = Reader.ReadRPCCount(Ctx);
	if (NewRPCCount.IsSet()
		&& NewRPCCount.GetValue() != State.LastRead)
	{
		uint64 RPCCount = NewRPCCount.GetValue();
		//TArray<RPCPayload>& Received = this->ReceivedRPCs.FindOrAdd(Ctx.EntityId);
		for (uint64 RPCId = State.LastRead + 1; RPCId <= RPCCount; ++RPCId)
		{
			uint32 Slot = (RPCId - 1) % NumberOfSlots;
			FRPCPayload NewPayload;
			Reader.ReadRPC(Ctx, Slot, NewPayload);
			//Received.Emplace(MoveTemp(NewPayload));
			QueueReceivedRPC(Ctx.EntityId, MoveTemp(NewPayload), RPCId);
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

void MonotonicRingBufferWithACKReceiver::ExtractReceivedRPCs(const RPCCallbacks::CanExtractRPCs& CanExtract, const ProcessRPC& Process)
{
	for (auto Iterator = ReceivedRPCs.CreateIterator(); Iterator; ++Iterator)
	{
		Worker_EntityId EntityId = Iterator->Key;
		if (!CanExtract(EntityId))
		{
			continue;
		}
		ReceiverState& State = ReceiverStates.FindChecked(EntityId);
		auto& Payloads = Iterator->Value;
		uint32 ProcessedRPCs = 0;
		for (const auto& Payload : Payloads)
		{
			const FRPCPayload& PayloadData = Payload.GetData();
			ReceivedRPC RPCToProcess = { PayloadData.Offset, PayloadData.Index, TArrayView<const uint8>(PayloadData.PayloadData) };
			if (Process(EntityId, RPCToProcess, Payload.GetAdditionalData()))
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
