// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriverRPC.h"
#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{
template <typename Payload, typename SerializerType>
class MonotonicRingBufferWithACKSender : public TRPCBufferSender<Payload>
{
	using Super = TRPCBufferSender<Payload>;
	using Super::ComponentsToReadOnAuthGained;
	using Super::ComponentsToReadOnUpdate;

public:
	MonotonicRingBufferWithACKSender(SerializerType&& InSerializer, int32 InNumberOfSlots)
		: Serializer(MoveTemp(InSerializer))
		, NumberOfSlots(InNumberOfSlots)
	{
		ComponentsToReadOnAuthGained.Add(Serializer.GetComponentId());
		ComponentsToReadOnAuthGained.Add(Serializer.GetACKComponentId());
		ComponentsToReadOnUpdate.Add(Serializer.GetACKComponentId());
	}

	virtual void OnUpdate(const RPCReadingContext& Ctx) override
	{
		if (Ctx.ComponentId == Serializer.GetACKComponentId())
		{
			BufferStateData& State = BufferState.FindChecked(Ctx.EntityId);

			TOptional<uint64> NewACKCount = Serializer.ReadACKCount(Ctx);
			if (NewACKCount)
			{
				State.LastACK = NewACKCount.GetValue();
			}
		}
	}

	virtual void OnAuthGained_ReadComponent(const RPCReadingContext& Ctx) override
	{
		if (Ctx.ComponentId == Serializer.GetComponentId())
		{
			OnAuthGained_ReadRPCComponent(Ctx);
		}
		if (Ctx.ComponentId == Serializer.GetACKComponentId())
		{
			OnAuthGained_ReadACKComponent(Ctx);
		}
	}

	void OnAuthGained_ReadRPCComponent(const RPCReadingContext& Ctx)
	{
		BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);
		TOptional<uint64> RPCCount = Serializer.ReadRPCCount(Ctx);
		State.CountWritten = RPCCount.Get(/*DefaultValue*/ 0);
	}

	void OnAuthGained_ReadACKComponent(const RPCReadingContext& Ctx)
	{
		BufferStateData& State = BufferState.FindOrAdd(Ctx.EntityId);
		TOptional<uint64> ACKCount = Serializer.ReadACKCount(Ctx);
		State.LastACK = ACKCount.Get(/*DefaultValue*/ 0);
	}

	virtual void OnAuthLost(Worker_EntityId Entity) override { BufferState.Remove(Entity); }

	virtual uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<const Payload> RPCs,
						 const RPCCallbacks::RPCWritten& WrittenCallback) override
	{
		BufferStateData& NextSlot = BufferState.FindOrAdd(EntityId);
		int32 AvailableSlots = FMath::Max(0, int32(NumberOfSlots) - int32(NextSlot.CountWritten - NextSlot.LastACK));

		int32 RPCsToWrite = FMath::Min(RPCs.Num(), AvailableSlots);
		if (RPCsToWrite > 0)
		{
			auto EntityWrite = Ctx.WriteTo(EntityId, Serializer.GetComponentId());
			for (int32 i = 0; i < RPCsToWrite; ++i)
			{
				uint64 RPCId = ++NextSlot.CountWritten;
				uint64 Slot = (RPCId - 1) % NumberOfSlots;
				Serializer.WriteRPC(EntityWrite, Slot, RPCs[i]);
				WrittenCallback(Serializer.GetComponentId(), RPCId);
			}
			Serializer.WriteRPCCount(EntityWrite, NextSlot.CountWritten);
		}

		return RPCsToWrite;
	}

private:
	struct BufferStateData
	{
		uint64 CountWritten = 0;
		uint64 LastACK = 0;
	};
	TMap<Worker_EntityId_Key, BufferStateData> BufferState;

	SerializerType Serializer;
	int32 NumberOfSlots;
};

} // namespace SpatialGDK
