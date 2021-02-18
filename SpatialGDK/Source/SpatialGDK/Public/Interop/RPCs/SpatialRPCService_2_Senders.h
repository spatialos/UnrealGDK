// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
class MonotonicRingBufferSender : public TRPCBufferSender<RPCPayload>
{
public:
	class ReaderInterface
	{
	public:
		virtual uint64 ReadRPCCount(RPCReadingContext&) = 0;
	};

	class WriterInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const RPCPayload& Payload) = 0;
		virtual void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) = 0;
	};

	MonotonicRingBufferSender(ReaderInterface& Reader, WriterInterface& Writer, int32 NumberOfSlots);

	void OnUpdate(RPCReadingContext&) override {}
	void OnAuthGained_ReadComponent(RPCReadingContext&) override;
	void OnAuthLost(Worker_EntityId Entity) override;
	uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, const TArray<RPCPayload>& RPCs) override;

private:
	TMap<Worker_EntityId_Key, uint64> BufferState;

	ReaderInterface& Reader;
	WriterInterface& Writer;
	int32 NumberOfSlots;
};

class SchemaMonotonicRingBufferSender : public MonotonicRingBufferSender
{
public:
	SchemaMonotonicRingBufferSender(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId,
									Schema_FieldId InFirstRingBufferSlotFieldId, uint32 InNumberOfSlots)
		: MonotonicRingBufferSender(ReaderImpl, WriterImpl, InNumberOfSlots)
		, ReaderImpl(InCountFieldId)
		, WriterImpl(InComponentId, InCountFieldId, InFirstRingBufferSlotFieldId)
	{
		ComponentsToReadOnAuthGained.Add(InComponentId);
	}

protected:
	class Reader : public ReaderInterface
	{
	public:
		Reader(Schema_FieldId InCountFieldId)
			: CountFieldId(InCountFieldId)
		{
		}
		uint64 ReadRPCCount(RPCReadingContext& Ctx) override { return Schema_GetUint64(Ctx.Fields, CountFieldId); }
		Schema_FieldId CountFieldId;
	};

	class Writer : public WriterInterface
	{
	public:
		Writer(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId, Schema_FieldId InFirstRingBufferSlotFieldId)
			: ComponentId(InComponentId)
			, CountFieldId(InCountFieldId)
			, FirstRingBufferSlotFieldId(InFirstRingBufferSlotFieldId)
		{
		}
		Worker_ComponentId GetComponentId() { return ComponentId; }
		void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const RPCPayload& Payload)
		{
			Schema_Object* NewField = Schema_AddObject(Ctx.GetFieldsToWrite(), Slot + FirstRingBufferSlotFieldId);
			Payload.WriteToSchemaObject(NewField);
			Ctx.RPCWritten(Slot);
		}
		void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count)
		{
			Schema_AddUint64(Ctx.GetFieldsToWrite(), CountFieldId, Count);
		}
		Worker_ComponentId ComponentId;
		Schema_FieldId CountFieldId;
		Schema_FieldId FirstRingBufferSlotFieldId;
	};
	Reader ReaderImpl;
	Writer WriterImpl;
};

class MonotonicRingBufferWithACKSender : public TRPCBufferSender<RPCPayload>
{
public:
	class ReaderInterface
	{
	public:
		virtual Worker_ComponentId GetACKComponentId() = 0;
		virtual uint64 ReadRPCCount(RPCReadingContext&) = 0;
		virtual uint64 ReadACKCount(RPCReadingContext&) = 0;
	};

	class WriterInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const RPCPayload& Payload) = 0;
		virtual void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) = 0;
	};

	MonotonicRingBufferWithACKSender(ReaderInterface& Reader, WriterInterface& Writer, int32 NumberOfSlots);

	void OnUpdate(RPCReadingContext&) override;
	void OnAuthGained_ReadWriteComponent(RPCReadingContext&);
	void OnAuthGained_ReadACKComponent(RPCReadingContext&);
	void OnAuthLost(Worker_EntityId Entity) override;
	uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, const TArray<RPCPayload>& RPCs) override;

private:
	struct BufferStateData
	{
		uint64 CountWritten = 0;
		uint64 LastACK = 0;
	};
	mutable TMap<Worker_EntityId_Key, BufferStateData> BufferState;

	ReaderInterface& Reader;
	WriterInterface& Writer;
	int32 NumberOfSlots;
};

class SchemaMonotonicRingBufferWithACKSender : public MonotonicRingBufferWithACKSender
{
public:
	SchemaMonotonicRingBufferWithACKSender(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId,
										   Schema_FieldId InFirstRingBufferSlotFieldId, uint32 InNumberOfSlots,
										   Worker_ComponentId InACKComponentId, Schema_FieldId InACKCountFieldId)
		: MonotonicRingBufferWithACKSender(ReaderImpl, WriterImpl, InNumberOfSlots)
		, ComponentId(InComponentId)
		, ACKComponentId(InACKComponentId)
		, ReaderImpl(InACKComponentId, InCountFieldId, InACKCountFieldId)
		, WriterImpl(InComponentId, InCountFieldId, InFirstRingBufferSlotFieldId)
	{
		ComponentsToReadOnAuthGained.Add(InComponentId);
		ComponentsToReadOnAuthGained.Add(InACKComponentId);
		ComponentsToReadOnUpdate.Add(InACKComponentId);
	}

	void OnAuthGained_ReadComponent(RPCReadingContext& Ctx) override
	{
		if (Ctx.ComponentId == ComponentId)
		{
			OnAuthGained_ReadWriteComponent(Ctx);
		}
		if (Ctx.ComponentId == ACKComponentId)
		{
			OnAuthGained_ReadACKComponent(Ctx);
		}
	}

protected:
	class Reader : public ReaderInterface
	{
	public:
		Reader(Worker_ComponentId InACKComponentId, Schema_FieldId InCountFieldId, Schema_FieldId InACKCountFieldId)
			: ACKComponentId(InACKComponentId)
			, CountFieldId(InCountFieldId)
			, ACKCountFieldId(InACKCountFieldId)
		{
		}
		Worker_ComponentId GetACKComponentId() override { return ACKComponentId; }
		uint64 ReadRPCCount(RPCReadingContext& Ctx) override { return Schema_GetUint64(Ctx.Fields, CountFieldId); }
		uint64 ReadACKCount(RPCReadingContext& Ctx) override { return Schema_GetUint64(Ctx.Fields, ACKCountFieldId); }
		Worker_ComponentId ACKComponentId;
		Schema_FieldId CountFieldId;
		Schema_FieldId ACKCountFieldId;
	};

	class Writer : public WriterInterface
	{
	public:
		Writer(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId, Schema_FieldId InFirstRingBufferSlotFieldId)
			: ComponentId(InComponentId)
			, CountFieldId(InCountFieldId)
			, FirstRingBufferSlotFieldId(InFirstRingBufferSlotFieldId)
		{
		}
		Worker_ComponentId GetComponentId() { return ComponentId; }
		void WriteRPC(RPCWritingContext::EntityWrite& Ctx, uint32 Slot, const RPCPayload& Payload)
		{
			Schema_Object* NewField = Schema_AddObject(Ctx.GetFieldsToWrite(), Slot + FirstRingBufferSlotFieldId);
			Payload.WriteToSchemaObject(NewField);
		}
		void WriteRPCCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count)
		{
			Schema_AddUint64(Ctx.GetFieldsToWrite(), CountFieldId, Count);
		}
		Worker_ComponentId ComponentId;
		Schema_FieldId CountFieldId;
		Schema_FieldId FirstRingBufferSlotFieldId;
	};
	Worker_ComponentId ComponentId;
	Worker_ComponentId ACKComponentId;
	Reader ReaderImpl;
	Writer WriterImpl;
};

} // namespace SpatialGDK
