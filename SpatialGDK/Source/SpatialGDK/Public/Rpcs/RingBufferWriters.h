// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include <improbable/c_worker.h>

#include "Schema/RPCPayload.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

namespace SpatialGDK
{
class CommandRequest;

class SpatialInterface
{
public:
	virtual ~SpatialInterface() = default;
	virtual void SendUpdate(Worker_EntityId EntityId, ComponentUpdate Update) = 0;
	virtual void SendCommand(Worker_EntityId EntityId, CommandRequest Request) = 0;
};

// Writes to a ring buffer with form:
// component RingBufferComponent {
//   id = ComponentId;
//   ...
//   int64 count = CountFieldId;
//   ...
//   option<Rpc> first = FirstRingBufferSlotId;
//   option<Rpc> second = FirstRingBufferSlotId + 1;
//   ...
//   option<Rpc> nth = FirstRingBufferSlotId + n - 1;
//   ...
// }
// count is incremented by 1 every time an RPC is sent.
// The ring buffer slots have a contiguous range of Field IDs starting from FirstRingBufferSlotId.
// If there are n slots, the kth RPC will be written to field ID: ((k - 1) % n)  + FirstRingBufferSlotFieldId.
// CountFieldId does not have to be smaller than FirstRingBufferSlotId.
class FMonotonicRingBufferWriter
{
public:
	explicit FMonotonicRingBufferWriter(Worker_ComponentId ComponentId, Schema_FieldId CountFieldId, Schema_FieldId FirstRingBufferSlotId,
										int32 NumberOfSlots)
		: ComponentId(ComponentId)
		, CountFieldId(CountFieldId)
		, FirstRingBufferSlotFieldId(FirstRingBufferSlotId)
		, NumberOfSlots(NumberOfSlots)
	{
	}

	ComponentUpdate CreateUpdate(const RPCPayload* Rpcs, int32 Count, int64 LastSentId) const
	{
		check(Count <= NumberOfSlots);
		ComponentUpdate Update(ComponentId);
		WriteToSchemaFields(Update.GetFields(), Rpcs, Count, LastSentId);
		return Update;
	}

	ComponentData CreateInitialData(const RPCPayload* Rpcs, int32 Count) const
	{
		check(Count <= NumberOfSlots);
		ComponentData Data(ComponentId);
		WriteToSchemaFields(Data.GetFields(), Rpcs, Count, 0);
		return Data;
	}

	int32 GetNumberOfSlots() const { return NumberOfSlots; }

private:
	void WriteToSchemaFields(Schema_Object* Fields, const RPCPayload* Rpcs, int32 Count, int64 LastSentId) const
	{
		Schema_AddInt64(Fields, CountFieldId, LastSentId + Count);
		for (int32 i = 0; i < Count; ++i)
		{
			const int64 RpcFieldId = ((LastSentId + i) % NumberOfSlots) + FirstRingBufferSlotFieldId;
			Schema_Object* RpcOptionField = Schema_AddObject(Fields, RpcFieldId);
			// Write RPC to field SlotId.
		}
	}

	Worker_ComponentId ComponentId;
	Schema_FieldId CountFieldId;
	Schema_FieldId FirstRingBufferSlotFieldId;
	int32 NumberOfSlots;
};

// Writes to a ring buffer with form:
// component Overflow {
//   id = ComponentId;
//   ...
//   list<Rpc> overflow_list = OverflowListFieldId;
//   ...
// }
class FOverflowBufferWriter
{
public:
	explicit FOverflowBufferWriter(Worker_ComponentId ComponentId, Schema_FieldId OverflowListFieldId)
		: ComponentId(ComponentId)
		, OverflowListFieldId(OverflowListFieldId)
	{
	}

	ComponentUpdate CreateUpdate(const RPCPayload* Rpcs, int32 Count) const
	{
		ComponentUpdate Update(ComponentId);
		if (Count == 0)
		{
			Schema_AddComponentUpdateClearedField(Update.GetUnderlying(), OverflowListFieldId);
		}
		else
		{
			WriteToSchemaFields(Update.GetFields(), Rpcs, Count);
		}
		return Update;
	}

	ComponentData CreateInitialData(const RPCPayload* Rpcs, int32 Count) const
	{
		ComponentData Data(ComponentId);
		WriteToSchemaFields(Data.GetFields(), Rpcs, Count);
		return Data;
	}

private:
	void WriteToSchemaFields(Schema_Object* Fields, const RPCPayload* Rpcs, int32 Count) const
	{
		for (int32 i = 0; i < Count; ++i) {}
	}

	Worker_ComponentId ComponentId;
	Schema_FieldId OverflowListFieldId;
};
} // namespace SpatialGDK
