// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriverRPC.h"
#include "Interop/RPCs/RPCTypes.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
template <typename PayloadType, template <typename> class PayloadWrapper, typename SerializerType>
class MulticastRingBufferReceiver : public TRPCBufferReceiver<PayloadType, PayloadWrapper>
{
	using Super = TRPCBufferReceiver<PayloadType, PayloadWrapper>;
	using Super::ComponentsToRead;
	using typename Super::ProcessRPC;

public:
	MulticastRingBufferReceiver(SerializerType&& InSerializer, int32 InNumberOfSlots, PayloadWrapper<PayloadType>&& InWrapper)
		: Super(MoveTemp(InWrapper))
		, Serializer(MoveTemp(InSerializer))
		, NumberOfSlots(InNumberOfSlots)
	{
		ComponentsToRead.Add(Serializer.GetComponentId());
	}

	virtual void OnAdded_ReadComponent(const RPCReadingContext& Ctx) override
	{
		ReceiverState& State = ReceiverStates.FindOrAdd(Ctx.EntityId);
		TOptional<uint64> RPCCount = Serializer.ReadRPCCount(Ctx);
		State.LastExecuted = RPCCount.Get(/*DefaultValue=*/0);
		State.LastRead = State.LastExecuted;

		ReadRPCs(Ctx, State);
	}

	virtual void OnRemoved(Worker_EntityId EntityId) override
	{
		ReceiverStates.Remove(EntityId);
		this->ReceivedRPCs.Remove(EntityId);
	}

	virtual void OnUpdate(const RPCReadingContext& Ctx) override
	{
		if (ComponentsToRead.Contains(Ctx.ComponentId))
		{
			ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
			ReadRPCs(Ctx, State);
		}
	}

	virtual void FlushUpdates(RPCWritingContext& Ctx) override {}

	virtual void ExtractReceivedRPCs(const RPCCallbacks::CanExtractRPCs& CanExtract, const ProcessRPC& Process) override
	{
		for (auto Iterator = this->ReceivedRPCs.CreateIterator(); Iterator; ++Iterator)
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
				const PayloadType& PayloadData = Payload.GetData();
				if (Process(EntityId, PayloadData, Payload.GetAdditionalData()))
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

protected:
	struct ReceiverState
	{
		uint64 LastRead;
		uint64 LastExecuted;
	};

	void ReadRPCs(const RPCReadingContext& Ctx, ReceiverState& State)
	{
		TOptional<uint64> NewRPCCount = Serializer.ReadRPCCount(Ctx);
		if (NewRPCCount.IsSet() && NewRPCCount.GetValue() != State.LastRead)
		{
			uint64 RPCCount = NewRPCCount.GetValue();
			for (uint64 RPCId = State.LastRead + 1; RPCId <= RPCCount; ++RPCId)
			{
				uint32 Slot = (RPCId - 1) % NumberOfSlots;
				PayloadType NewPayload;
				Serializer.ReadRPC(Ctx, Slot, NewPayload);
				this->QueueReceivedRPC(Ctx.EntityId, MoveTemp(NewPayload), RPCId);
			}
			State.LastRead = RPCCount;
		}
	}

	TMap<Worker_EntityId_Key, ReceiverState> ReceiverStates;

	SerializerType Serializer;
	int32 NumberOfSlots;
};

} // namespace SpatialGDK
