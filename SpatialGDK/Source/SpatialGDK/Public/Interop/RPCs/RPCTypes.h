// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/EntityView.h"

namespace SpatialGDK
{
struct RPCReadingContext
{
	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;

	bool IsUpdate() const { return Update != nullptr; }

	Schema_ComponentUpdate* Update = nullptr;
	Schema_Object* Fields = nullptr;
};

struct RPCWritingContext
{
	enum class DataKind
	{
		Generic,
		Data,
		Update,
		CommandRequest,
		CommandResponse
	};

	class EntityWrite
	{
	public:
		EntityWrite(const EntityWrite&) = delete;
		EntityWrite& operator=(const EntityWrite&) = delete;
		EntityWrite& operator=(EntityWrite&&) = delete;

		EntityWrite(EntityWrite&& Write);
		~EntityWrite();

		Schema_ComponentUpdate* GetComponentUpdateToWrite();
		Schema_Object* GetFieldsToWrite();

		void RPCWritten(uint32 RPCId);

		const Worker_EntityId EntityId;
		const Worker_ComponentId ComponentId;

	private:
		union
		{
			Schema_GenericData* GenData;
			Schema_ComponentData* Data;
			Schema_ComponentUpdate* Update;
			Schema_CommandRequest* Request;
			Schema_CommandResponse* Response;
		};

		EntityWrite(RPCWritingContext& InCtx, Worker_EntityId InEntityId, Worker_ComponentId InComponentID);
		friend RPCWritingContext;
		RPCWritingContext& Ctx;
		Schema_Object* Fields = nullptr;
		bool bActiveWriter = true;
	};

	EntityWrite WriteTo(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	RPCWritingContext(DataKind InKind);

	TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentData*)> DataWrittenCallback;
	TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentUpdate*)> UpdateWrittenCallback;
	TFunction<void(Worker_EntityId, Schema_CommandRequest*)> RequestWrittenCallback;
	TFunction<void(Worker_EntityId, Schema_CommandResponse*)> ResponseWrittenCallback;

	TFunction<void(Worker_EntityId, Worker_ComponentId, uint32)> RPCWrittenCallback;

protected:
	DataKind Kind;
};

class RPCBufferSender
{
public:
	virtual ~RPCBufferSender() = default;

	virtual void OnUpdate(RPCReadingContext& iCtx) = 0;
	virtual void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element);
	virtual void OnAuthGained_ReadComponent(RPCReadingContext& iCtx) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;

protected:
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
	TSet<Worker_ComponentId> ComponentsToReadOnUpdate;
};

using CanExtractRPCs = TFunction<bool(Worker_EntityId)>;
using ProcessRPC = TFunction<bool(Worker_EntityId, const RPCPayload&)>;

class RPCBufferReceiver
{
public:
	virtual ~RPCBufferReceiver() = default;

	virtual void OnAdded(Worker_EntityId EntityId, EntityViewElement const& Element);
	virtual void OnAdded_ReadComponent(RPCReadingContext& iCtx) = 0;
	virtual void OnRemoved(Worker_EntityId EntityId) = 0;
	virtual void OnUpdate(RPCReadingContext& iCtx) = 0;
	virtual void FlushUpdates(RPCWritingContext&) = 0;
	virtual void ExtractReceivedRPCs(const CanExtractRPCs&, const ProcessRPC&) = 0;

protected:
	TSet<Worker_ComponentId> ComponentsToRead;
};

struct RPCQueue
{
	virtual ~RPCQueue() = default;
	virtual void FlushAll(RPCWritingContext&) = 0;
	virtual void Flush(Worker_EntityId EntityId, RPCWritingContext&) = 0;
	virtual void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element);
	virtual void OnAuthGained_ReadComponent(RPCReadingContext& iCtx) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;

protected:
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
};

template <typename PayloadType>
struct TRPCBufferSender : RPCBufferSender
{
	virtual uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<PayloadType> RPCs) = 0;
};

template <typename PayloadType>
struct TRPCQueue : RPCQueue
{
	TRPCQueue(TRPCBufferSender<PayloadType>& InSender)
		: Sender(InSender)
	{
	}

	virtual void Push(Worker_EntityId EntityId, PayloadType&& Payload)
	{
		auto& Queue = Queues.FindOrAdd(EntityId);
		if (Queue.bAdded)
		{
			Queue.RPCs.Emplace(MoveTemp(Payload));
		}
	}

	void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element) override
	{
		RPCQueue::OnAuthGained(EntityId, Element);
		Queues.FindOrAdd(EntityId).bAdded = true;
	}

	struct QueueData
	{
		TArray<PayloadType, TInlineAllocator<1>> RPCs;
		bool bAdded = false;
	};

	TMap<Worker_EntityId, QueueData> Queues;
	TRPCBufferSender<PayloadType>& Sender;
};

} // namespace SpatialGDK
