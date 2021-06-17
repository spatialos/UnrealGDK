// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"

namespace SpatialGDK
{
template <typename Payload>
class MulticastRingBufferSerializer_Schema
{
public:
	MulticastRingBufferSerializer_Schema(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId,
										 Schema_FieldId InInitialCountFieldId, Schema_FieldId InFirstRingBufferSlotFieldId)
		: ComponentId(InComponentId)
		, CountFieldId(InCountFieldId)
		, InitialCountFieldId(InInitialCountFieldId)
		, FirstRingBufferSlotFieldId(InFirstRingBufferSlotFieldId)
	{
		check(ComponentId != 0);
	}

	Worker_ComponentId GetComponentId() { return ComponentId; }

	TOptional<uint64> ReadRPCCount(const RPCReadingContext& Ctx)
	{
		if (Schema_GetUint64Count(Ctx.Fields, CountFieldId))
		{
			return Schema_GetUint64(Ctx.Fields, CountFieldId);
		}
		return {};
	}

	TOptional<uint32> ReadInitialRPCCount(const RPCReadingContext& Ctx)
	{
		if (Schema_GetUint32Count(Ctx.Fields, InitialCountFieldId))
		{
			return Schema_GetUint32(Ctx.Fields, InitialCountFieldId);
		}
		return {};
	}

	void ReadRPC(const RPCReadingContext& Ctx, uint32 Slot, Payload& OutPayload)
	{
		Schema_Object* PayloadObject = Schema_GetObject(Ctx.Fields, FirstRingBufferSlotFieldId + Slot);
		if (ensure(PayloadObject))
		{
			OutPayload.ReadFromSchema(PayloadObject);
		}
	}

	void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const Payload& InPayload)
	{
		Schema_Object* NewField = Schema_AddObject(Ctx.GetFieldsToWrite(), Slot + FirstRingBufferSlotFieldId);
		InPayload.WriteToSchema(NewField);
	}

	void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) { Schema_AddUint64(Ctx.GetFieldsToWrite(), CountFieldId, Count); }

	void WriteInitialRPCCount(RPCWritingContext::EntityWrite& Ctx, uint32 Count)
	{
		Schema_AddUint32(Ctx.GetFieldsToWrite(), CountFieldId, Count);
	}

private:
	const Worker_ComponentId ComponentId;
	const Schema_FieldId CountFieldId;
	const Schema_FieldId InitialCountFieldId;
	const Schema_FieldId FirstRingBufferSlotFieldId;
};

} // namespace SpatialGDK
