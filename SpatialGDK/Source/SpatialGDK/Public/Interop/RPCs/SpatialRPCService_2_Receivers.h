// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/RPCs/RPCTypes.h"
#include "EngineClasses/SpatialNetDriverRPC.h"

namespace SpatialGDK
{

//template <typename Wrapper>
class MonotonicRingBufferWithACKReceiver : public TRPCBufferReceiver<FRPCPayload, /*Wrapper*/TimestampAndETWrapper>
{
public:
	//template <typename T>
	//using Wrapper = TimestampAndETWrapper<T>;

	class ReaderInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual TOptional<uint64> ReadRPCCount(const RPCReadingContext&) = 0;
		virtual TOptional<uint64> ReadACKCount(const RPCReadingContext&) = 0;
		virtual void ReadRPC(const RPCReadingContext& Ctx, uint32 Slot, FRPCPayload& Payload) = 0;
	};

	class WriterInterface
	{
	public:
		virtual Worker_ComponentId GetComponentId() = 0;
		virtual void WriteACKCount(RPCWritingContext::EntityWrite& Ctx, uint64 Count) = 0;
	};

	MonotonicRingBufferWithACKReceiver(TimestampAndETWrapper<FRPCPayload>&& Wrapper, ReaderInterface& Reader, WriterInterface& Writer, int32 NumberOfSlots);

	void OnAdded(FName ReceiverName, Worker_EntityId EntityId, EntityViewElement const& Element) override;
	void OnAdded_ReadRPCComponent(const RPCReadingContext&);
	void OnAdded_ReadACKComponent(const RPCReadingContext&);
	void OnRemoved(Worker_EntityId EntityId) override;
	void OnUpdate(const RPCReadingContext& iCtx) override;
	void FlushUpdates(RPCWritingContext&) override;
	void ExtractReceivedRPCs(const RPCCallbacks::CanExtractRPCs&, const ProcessRPC&) override;

protected:
	struct ReceiverState
	{
		uint64 LastRead;
		uint64 LastWrittenACK;
		uint64 LastExecuted;
	};

	void ReadRPCs(const RPCReadingContext&, ReceiverState&);

	TMap<Worker_EntityId_Key, ReceiverState> ReceiverStates;
	//TMap<Worker_EntityId_Key, TArray<RPCPayload>> ReceivedRPCs;

	ReaderInterface& Reader;
	WriterInterface& Writer;
	int32 NumberOfSlots;
};

class SchemaMonotonicRingBufferWithACKReceiver : public MonotonicRingBufferWithACKReceiver
{
public:
	SchemaMonotonicRingBufferWithACKReceiver(TimestampAndETWrapper<FRPCPayload>&& Wrapper, Worker_ComponentId InComponentId, Schema_FieldId InCountFieldId,
											 Schema_FieldId InFirstRingBufferSlotFieldId, uint32 InNumberOfSlots,
											 Worker_ComponentId InACKComponentId, Schema_FieldId InACKCountFieldId)
		: MonotonicRingBufferWithACKReceiver(MoveTemp(Wrapper), ReaderImpl, WriterImpl, InNumberOfSlots)
		, ComponentId(InComponentId)
		, ACKComponentId(InACKComponentId)
		, ReaderImpl(InComponentId, InCountFieldId, InACKCountFieldId, InFirstRingBufferSlotFieldId)
		, WriterImpl(InACKComponentId, InACKCountFieldId)
	{
		ComponentsToRead.Add(InComponentId);
		ComponentsToRead.Add(InACKComponentId);
	}

	void OnAdded_ReadComponent(const RPCReadingContext& Ctx) override
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
		TOptional<uint64> ReadRPCCount(const RPCReadingContext& Ctx) override
		{
			if (Schema_GetUint64Count(Ctx.Fields, CountFieldId))
			{
				return Schema_GetUint64(Ctx.Fields, CountFieldId);
			}
			return {};
		}
		TOptional<uint64> ReadACKCount(const RPCReadingContext& Ctx) override
		{
			if (Schema_GetUint64Count(Ctx.Fields, ACKCountFieldId))
			{
				return Schema_GetUint64(Ctx.Fields, ACKCountFieldId);
			}
			return {};
		}
		void ReadRPC(const RPCReadingContext& Ctx, uint32 Slot, FRPCPayload& Payload) override
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
