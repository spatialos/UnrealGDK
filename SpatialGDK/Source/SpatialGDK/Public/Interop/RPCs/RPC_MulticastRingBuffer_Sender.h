// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetDriverRPC.h"
#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{
template <typename Payload, typename SerializerType>
class MulticastRingBufferSender : public TRPCBufferSender<Payload>
{
	using Super = TRPCBufferSender<Payload>;
	using Super::ComponentsToReadOnAuthGained;
	using Super::ComponentsToReadOnUpdate;

public:
	MulticastRingBufferSender(SerializerType&& InSerializer, int32 InNumberOfSlots)
		: Serializer(MoveTemp(InSerializer))
		, NumberOfSlots(InNumberOfSlots)
	{
		ComponentsToReadOnAuthGained.Add(Serializer.GetComponentId());
	}

	virtual void OnUpdate(const RPCReadingContext& Ctx) override {}

	virtual void OnAuthGained_ReadComponent(const RPCReadingContext& Ctx) override
	{
		if (Ctx.ComponentId == Serializer.GetComponentId())
		{
			TOptional<uint64> Written = Serializer.ReadRPCCount(Ctx);
			TOptional<uint32> InitiallyPresent = Serializer.ReadInitialRPCCount(Ctx);

			BufferStateData Buffer;
			if (Written && InitiallyPresent && Written.GetValue() == 0 && InitiallyPresent.GetValue() > 0)
			{
				Buffer.CountWritten = InitiallyPresent.GetValue();
				DirtyBuffer.Add(Ctx.EntityId);
			}
			else
			{
				Buffer.CountWritten = Written.Get(/*DefaultValue=*/0);
			}
			BufferState.Add(Ctx.EntityId, Buffer);
		}
	}

	virtual void OnAuthLost(Worker_EntityId Entity) override
	{
		DirtyBuffer.Remove(Entity);
		BufferState.Remove(Entity);
	}

	virtual uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<const Payload> RPCs,
						 const RPCCallbacks::RPCWritten& WrittenCallback) override
	{
		BufferStateData& NextSlot = BufferState.FindOrAdd(EntityId);
		int32 AvailableSlots = NumberOfSlots;

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
			if (Ctx.GetDataKind() == RPCWritingContext::DataKind::ComponentData)
			{
				Serializer.WriteInitialRPCCount(EntityWrite, NextSlot.CountWritten);
			}
		}

		return RPCsToWrite;
	}

	virtual void FlushUpdates(RPCWritingContext& Ctx) override
	{
		for (Worker_EntityId EntityId : DirtyBuffer)
		{
			BufferStateData* StateData = BufferState.Find(EntityId);
			if (ensure(StateData != nullptr))
			{
				auto EntityWrite = Ctx.WriteTo(EntityId, Serializer.GetComponentId());
				Serializer.WriteRPCCount(EntityWrite, StateData->CountWritten);
			}
		}
		DirtyBuffer.Empty();
	}

private:
	struct BufferStateData
	{
		uint64 CountWritten = 0;
	};
	TMap<Worker_EntityId_Key, BufferStateData> BufferState;
	TSet<Worker_EntityId_Key> DirtyBuffer;

	SerializerType Serializer;
	int32 NumberOfSlots;
};

} // namespace SpatialGDK
