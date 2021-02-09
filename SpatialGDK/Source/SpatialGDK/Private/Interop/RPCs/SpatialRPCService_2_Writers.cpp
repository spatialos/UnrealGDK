#include "Interop/RPCs/SpatialRPCService_2_Writers.h"

namespace SpatialGDK
{
MonotonicRingBufferWriter::MonotonicRingBufferWriter(Worker_ComponentId ComponentId, Schema_FieldId CountFieldId,
													 Schema_FieldId FirstRingBufferSlotId, int32 NumberOfSlots)
	: ComponentId(ComponentId)
	, CountFieldId(CountFieldId)
	, FirstRingBufferSlotFieldId(FirstRingBufferSlotId)
	, NumberOfSlots(NumberOfSlots)
{
}

Worker_ComponentId MonotonicRingBufferWriter::GetComponentToWriteTo() const
{
	return ComponentId;
}

uint32 MonotonicRingBufferWriter::GetFreeSlots(Worker_EntityId) const
{
	return NumberOfSlots;
}

void MonotonicRingBufferWriter::OnAuthGained(Worker_EntityId Entity, EntityViewElement const& Element)
{
	const ComponentData* Data = Element.Components.FindByPredicate([this](ComponentData const& Data) {
		return Data.GetComponentId() == ComponentId;
	});
	Schema_Object* DataObject = Schema_GetComponentDataFields(Data->GetUnderlying());
	uint64 CounterValue = Schema_GetUint64(DataObject, CountFieldId);

	BufferState.Add(Entity, CounterValue);
}

void MonotonicRingBufferWriter::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferWriter::Write(Worker_EntityId EntityId, Schema_Object* ComponentObject, const uint8* RPCObjectBytes,
										uint32 RPCObjectSize)
{
	uint64& NextSlot = BufferState.FindOrAdd(EntityId, 0);
	uint64 Count = ++NextSlot;
	uint64 Slot = (Count) % NumberOfSlots;
	Schema_Object* NewField = Schema_AddObject(ComponentObject, Slot + FirstRingBufferSlotFieldId);
	Schema_AllocateBuffer(NewField, RPCObjectSize);
	Schema_MergeFromBuffer(NewField, RPCObjectBytes, RPCObjectSize);
	Schema_ClearField(ComponentObject, CountFieldId);
	Schema_AddUint64(ComponentObject, CountFieldId, Count);

	return Slot;
}

MonotonicRingBufferWithACKWriter::MonotonicRingBufferWithACKWriter(Worker_ComponentId ComponentId, Schema_FieldId CountFieldId,
																   Schema_FieldId FirstRingBufferSlotId, int32 NumberOfSlots,
																   Worker_ComponentId InACKComponentId, Schema_FieldId InACKCountFieldId)
	: ComponentId(ComponentId)
	, CountFieldId(CountFieldId)
	, FirstRingBufferSlotFieldId(FirstRingBufferSlotId)
	, NumberOfSlots(NumberOfSlots)
	, ACKComponentId(InACKComponentId)
	, ACKCountFieldId(InACKCountFieldId)
{
}

Worker_ComponentId MonotonicRingBufferWithACKWriter::GetComponentToWriteTo() const
{
	return ComponentId;
}

uint32 MonotonicRingBufferWithACKWriter::GetFreeSlots(Worker_EntityId EntityId) const
{
	BufferStateData& NextSlot = BufferState.FindOrAdd(EntityId, BufferStateData());
	return NumberOfSlots - (NextSlot.CountWritten - NextSlot.LastACK);
}

void MonotonicRingBufferWithACKWriter::OnUpdate(Worker_EntityId EntityId, const ComponentChange& Update)
{
	if (Update.ComponentId == ACKComponentId)
	{
		BufferStateData& State = BufferState.FindChecked(EntityId);
		Schema_Object* ACKDataObject = Schema_GetComponentUpdateFields(Update.Update);
		if (Schema_GetInt64Count(ACKDataObject, ACKCountFieldId) > 0)
		{
			State.LastACK = Schema_GetUint64(ACKDataObject, ACKCountFieldId);
		}
	}
}

void MonotonicRingBufferWithACKWriter::OnCompleteUpdate(Worker_EntityId EntityId, const ComponentChange& Update)
{
	if (Update.ComponentId == ACKComponentId)
	{
		BufferStateData& State = BufferState.FindChecked(EntityId);
		Schema_Object* ACKDataObject = Schema_GetComponentDataFields(Update.CompleteUpdate.Data);
		State.LastACK = Schema_GetUint64(ACKDataObject, ACKCountFieldId);
	}
}

void MonotonicRingBufferWithACKWriter::OnAuthGained(Worker_EntityId Entity, EntityViewElement const& Element)
{
	BufferStateData State;
	{
		const ComponentData* WriteData = Element.Components.FindByPredicate([this](ComponentData const& Data) {
			return Data.GetComponentId() == ComponentId;
		});

		Schema_Object* DataObject = Schema_GetComponentDataFields(WriteData->GetUnderlying());
		State.CountWritten = Schema_GetUint64(DataObject, CountFieldId);
	}

	{
		const ComponentData* ACKData = Element.Components.FindByPredicate([this](ComponentData const& Data) {
			return Data.GetComponentId() == ACKComponentId;
		});
		Schema_Object* ACKDataObject = Schema_GetComponentDataFields(ACKData->GetUnderlying());
		State.LastACK = Schema_GetUint64(ACKDataObject, ACKCountFieldId);
	}

	BufferState.Add(Entity, State);
}

void MonotonicRingBufferWithACKWriter::OnAuthLost(Worker_EntityId Entity)
{
	BufferState.Remove(Entity);
}

uint32 MonotonicRingBufferWithACKWriter::Write(Worker_EntityId EntityId, Schema_Object* ComponentObject, const uint8* RPCObjectBytes,
											   uint32 RPCObjectSize)
{
	BufferStateData& Data = BufferState.FindOrAdd(EntityId, BufferStateData());
	uint64 Count = ++Data.CountWritten;
	uint64 Slot = (Count) % NumberOfSlots;
	// More efficient, but told it's a bit of a hack.
	// Schema_AddBytes(ComponentObject, Slot + FirstRingBufferSlotFieldId, RPCObjectBytes, RPCObjectSize);
	Schema_Object* NewField = Schema_AddObject(ComponentObject, Slot + FirstRingBufferSlotFieldId);
	Schema_AllocateBuffer(NewField, RPCObjectSize);

	Schema_MergeFromBuffer(NewField, RPCObjectBytes, RPCObjectSize);
	Schema_ClearField(ComponentObject, CountFieldId);
	Schema_AddUint64(ComponentObject, CountFieldId, Count);

	return Slot;
}
} // namespace SpatialGDK
