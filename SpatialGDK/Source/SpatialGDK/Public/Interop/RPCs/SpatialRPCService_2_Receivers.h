// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/SpatialRPCService_2.h"

namespace SpatialGDK
{
class MonotonicRingBufferWithACKReceiver : public RPCBufferReceiver
{
public:
	class ReaderInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual uint64 ReadRPCCount(RPCReadingContext&) = 0;
		virtual uint64 ReadACKCount(RPCReadingContext&) = 0;
		virtual void ReadRPC(RPCReadingContext& Ctx, uint32 Slot, RPCPayload& Payload) = 0;
	};

	class WriterInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual void WriteACKCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) = 0;
	};

	MonotonicRingBufferWithACKReceiver(ReaderInterface& Reader, WriterInterface& Writer, int32 NumberOfSlots);

	void OnAdded(Worker_EntityId EntityId, EntityViewElement const& Element) override;
	void OnAdded_ReadRPCComponent(RPCReadingContext&);
	void OnAdded_ReadACKComponent(RPCReadingContext&);
	void OnRemoved(Worker_EntityId EntityId) override;
	void OnUpdate(RPCReadingContext& iCtx) override;
	void FlushUpdates(RPCWritingContext&) override;
	void ExtractReceivedRPCs(const CanExtractRPCs&, const ProcessRPC&) override;

protected:
	struct ReceiverState
	{
		uint64 LastRead;
		uint64 LastWrittenACK;
		uint64 LastExecuted;
	};

	void ReadRPCs(RPCReadingContext&, ReceiverState&);

	TMap<Worker_EntityId, ReceiverState> ReceiverStates;
	TMap<Worker_EntityId, TArray<RPCPayload>> ReceivedRPCs;

	ReaderInterface& Reader;
	WriterInterface& Writer;
	int32 NumberOfSlots;
};

class SchemaMonotonicRingBufferWithACKReceiver : public MonotonicRingBufferWithACKReceiver
{
public:
	SchemaMonotonicRingBufferWithACKReceiver(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId,
											 Schema_FieldId InFirstRingBufferSlotFieldId, uint32 InNumberOfSlots,
											 Worker_ComponentId InACKComponentId, Schema_FieldId InACKCountFieldId)
		: MonotonicRingBufferWithACKReceiver(ReaderImpl, WriterImpl, InNumberOfSlots)
		, ComponentId(InComponentId)
		, ACKComponentId(InACKComponentId)
		, ReaderImpl(InComponentId, InCountFieldId, InACKCountFieldId, InFirstRingBufferSlotFieldId)
		, WriterImpl(InACKComponentId, InACKCountFieldId)
	{
		ComponentsToRead.Add(InComponentId);
		ComponentsToRead.Add(InACKComponentId);
	}

	void OnAdded_ReadComponent(RPCReadingContext& Ctx) override
	{
		if (Ctx.ComponentId == ComponentId)
		{
			OnAdded_ReadRPCComponent(Ctx);
		}
		if (Ctx.ComponentId == ACKComponentId)
		{
			OnAdded_ReadACKComponent(Ctx);
		}
	}

protected:
	class Reader : public ReaderInterface
	{
	public:
		Reader(Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId, Schema_FieldId InACKCountFieldId,
			   Schema_FieldId InFirstRingBufferSlotFieldId)
			: CountFieldId(InCountFieldId)
			, ACKCountFieldId(InACKCountFieldId)
			, FirstRingBufferSlotFieldId(InFirstRingBufferSlotFieldId)
			, ComponentId(InComponentId)
		{
		}
		Worker_ComponentId GetComponentId() override { return ComponentId; };
		uint64 ReadRPCCount(RPCReadingContext& Ctx) override { return Schema_GetUint64(Ctx.Fields, CountFieldId); }
		uint64 ReadACKCount(RPCReadingContext& Ctx) override { return Schema_GetUint64(Ctx.Fields, ACKCountFieldId); }
		void ReadRPC(RPCReadingContext& Ctx, uint32 Slot, RPCPayload& Payload) override
		{
			Schema_Object* PayloadObject = Schema_GetObject(Ctx.Fields, FirstRingBufferSlotFieldId + Slot);
			if (ensure(PayloadObject))
			{
				Payload.ReadFromSchema(PayloadObject);
			}
		}
		Schema_FieldId CountFieldId;
		Schema_FieldId ACKCountFieldId;
		Schema_FieldId FirstRingBufferSlotFieldId;
		Worker_ComponentId ComponentId;
	};

	class Writer : public WriterInterface
	{
	public:
		Writer(Worker_ComponentId InACKComponentId, Schema_FieldId InCountFieldId)
			: ACKCountFieldId(InCountFieldId)
			, ACKComponentId(InACKComponentId)

		{
		}
		Worker_ComponentId GetComponentId() override { return ACKComponentId; }
		void WriteACKCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) override
		{
			Schema_AddUint64(Ctx.GetFieldsToWrite(), ACKCountFieldId, Count);
		}
		Schema_FieldId ACKCountFieldId;
		Worker_ComponentId ACKComponentId;
	};
	Worker_ComponentId ComponentId;
	Worker_ComponentId ACKComponentId;
	Reader ReaderImpl;
	Writer WriterImpl;
};

} // namespace SpatialGDK
