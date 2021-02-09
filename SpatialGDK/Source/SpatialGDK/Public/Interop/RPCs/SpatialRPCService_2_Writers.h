// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
class MonotonicRingBufferWriter : public RPCWriter
{
public:
	explicit MonotonicRingBufferWriter(Worker_ComponentId ComponentId, Schema_FieldId CountFieldId, Schema_FieldId FirstRingBufferSlotId,
									   int32 NumberOfSlots);

	Worker_ComponentId GetComponentToWriteTo() const override;
	uint32 GetFreeSlots(Worker_EntityId) const override;
	void OnUpdate(Worker_EntityId, const ComponentChange& Update) override {}
	void OnCompleteUpdate(Worker_EntityId, const ComponentChange& Update) override {}
	void OnAuthGained(Worker_EntityId Entity, EntityViewElement const& Element) override;
	void OnAuthLost(Worker_EntityId Entity) override;
	uint32 Write(Worker_EntityId EntityId, Schema_Object* ComponentObject, const uint8* RPCObjectBytes, uint32 RPCObjectSize) override;

private:
	TMap<Worker_EntityId_Key, uint64> BufferState;

	Worker_ComponentId ComponentId;
	Schema_FieldId CountFieldId;
	Schema_FieldId FirstRingBufferSlotFieldId;
	int32 NumberOfSlots;
};

class MonotonicRingBufferWithACKWriter : public RPCWriter
{
public:
	explicit MonotonicRingBufferWithACKWriter(Worker_ComponentId ComponentId, Schema_FieldId CountFieldId,
											  Schema_FieldId FirstRingBufferSlotId, int32 NumberOfSlots,
											  Worker_ComponentId InACKComponentId, Schema_FieldId InACKCountFieldId);

	Worker_ComponentId GetComponentToWriteTo() const override;
	uint32 GetFreeSlots(Worker_EntityId EntityId) const override;
	void OnUpdate(Worker_EntityId EntityId, const ComponentChange& Update) override;
	void OnCompleteUpdate(Worker_EntityId EntityId, const ComponentChange& Update) override;
	void OnAuthGained(Worker_EntityId Entity, EntityViewElement const& Element) override;
	void OnAuthLost(Worker_EntityId Entity) override;
	uint32 Write(Worker_EntityId EntityId, Schema_Object* ComponentObject, const uint8* RPCObjectBytes, uint32 RPCObjectSize) override;

private:
	struct BufferStateData
	{
		uint64 CountWritten = 0;
		uint64 LastACK = 0;
	};
	mutable TMap<Worker_EntityId_Key, BufferStateData> BufferState;

	Worker_ComponentId ComponentId;
	Schema_FieldId CountFieldId;
	Schema_FieldId FirstRingBufferSlotFieldId;
	int32 NumberOfSlots;
	Worker_ComponentId ACKComponentId;
	Schema_FieldId ACKCountFieldId;
};

} // namespace SpatialGDK
