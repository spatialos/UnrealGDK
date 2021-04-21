// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriverRPC.h"
#include "Interop/RPCs/RPCTypes.h"
#include "SpatialView/EntityComponentTypes.h"

namespace SpatialGDK
{
template <typename PayloadType, template <typename> class PayloadWrapper, typename SerializerType>
class MonotonicRingBufferWithACKReceiver : public TRPCBufferReceiver<PayloadType, PayloadWrapper>
{
	using Super = TRPCBufferReceiver<PayloadType, PayloadWrapper>;
	using Super::ComponentsToRead;
	using typename Super::ProcessRPC;

public:
	MonotonicRingBufferWithACKReceiver(SerializerType&& InSerializer, int32 InNumberOfSlots, PayloadWrapper<PayloadType>&& InWrapper)
		: Super(MoveTemp(InWrapper))
		, Serializer(MoveTemp(InSerializer))
		, NumberOfSlots(InNumberOfSlots)
	{
		ComponentsToRead.Add(Serializer.GetComponentId());
		ComponentsToRead.Add(Serializer.GetACKComponentId());
	}

	virtual void OnAdded(FName ReceiverName, Worker_EntityId EntityId, EntityViewElement const& Element) override
	{
		const ComponentData* RPCComponentData = Element.Components.FindByPredicate(ComponentIdEquality{ Serializer.GetComponentId() });
		const ComponentData* ACKComponentData = Element.Components.FindByPredicate(ComponentIdEquality{ Serializer.GetACKComponentId() });
		RPCReadingContext ReadCtx;
		ReadCtx.ReaderName = ReceiverName;
		ReadCtx.EntityId = EntityId;

		// Making sure we read component data in order, first ACK, then RPC
		// to know which RPC we actually need to extract
		if (ensure(ACKComponentData != nullptr))
		{
			ReadCtx.ComponentId = Serializer.GetComponentId();
			ReadCtx.Fields = Schema_GetComponentDataFields(RPCComponentData->GetUnderlying());
			OnAdded_ReadRPCComponent(ReadCtx);
		}

		if (ensure(RPCComponentData != nullptr))
		{
			ReadCtx.ComponentId = Serializer.GetACKComponentId();
			ReadCtx.Fields = Schema_GetComponentDataFields(ACKComponentData->GetUnderlying());
			OnAdded_ReadACKComponent(ReadCtx);
		}
	}

	virtual void OnAdded_ReadComponent(const RPCReadingContext& Ctx) override
	{
		checkNoEntry();
	}

	void OnAdded_ReadRPCComponent(const RPCReadingContext& Ctx)
	{
		ReceiverState& State = ReceiverStates.FindChecked(Ctx.EntityId);
		ReadRPCs(Ctx, State);
	}

	void OnAdded_ReadACKComponent(const RPCReadingContext& Ctx)
	{
		ReceiverState& State = ReceiverStates.FindOrAdd(Ctx.EntityId);
		TOptional<uint64> ACKCount = Serializer.ReadACKCount(Ctx);
		State.LastExecuted = ACKCount ? ACKCount.GetValue() : 0;
		State.LastRead = State.LastExecuted;
		State.LastWrittenACK = State.LastExecuted;
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

	virtual void FlushUpdates(RPCWritingContext& Ctx) override
	{
		for (auto& Receiver : ReceiverStates)
		{
			Worker_EntityId EntityId = Receiver.Key;
			ReceiverState& State = Receiver.Value;
			if (State.LastExecuted != State.LastWrittenACK)
			{
				auto EntityWriter = Ctx.WriteTo(EntityId, Serializer.GetACKComponentId());
				Serializer.WriteACKCount(EntityWriter, State.LastExecuted);
				State.LastWrittenACK = State.LastExecuted;
			}
		}
	}

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
		uint64 LastWrittenACK;
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
