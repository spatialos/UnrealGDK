// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialGDKSpanId.h"

#include "Schema/ClientEndpoint.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/SubView.h"
#include "Utils/RPCContainer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService_2, Log, All);

class USpatialNetDriver;

namespace SpatialGDK
{
struct RPCReadingContext
{
	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;

	bool IsUpdate() const { return Update != nullptr; }

	Schema_ComponentUpdate* Update = nullptr;
	Schema_Object* Fields = nullptr;

	TFunction<void(RPCPayload const&)> RPCReceivedCallback;
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
	virtual void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element)
	{
		RPCReadingContext readCtx;
		readCtx.EntityId = EntityId;
		for (const auto& Component : Element.Components)
		{
			if (ComponentsToReadOnAuthGained.Contains(Component.GetComponentId()))
			{
				readCtx.ComponentId = Component.GetComponentId();
				readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

				OnAuthGained_ReadComponent(readCtx);
			}
		}
	}

	virtual void OnAuthGained_ReadComponent(RPCReadingContext& iCtx) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;

protected:
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
	TSet<Worker_ComponentId> ComponentsToReadOnUpdate;
};

using CanExtractRPCs = TFunction<bool(Worker_EntityId)>;
using ProcessRPC = TFunction<bool(const FUnrealObjectRef&, const SpatialGDK::RPCPayload&)>;

class RPCBufferReceiver
{
public:
	virtual ~RPCBufferReceiver() = default;

	virtual void OnAdded(Worker_EntityId EntityId, EntityViewElement const& Element)
	{
		RPCReadingContext readCtx;
		readCtx.EntityId = EntityId;
		for (const auto& Component : Element.Components)
		{
			if (ComponentsToRead.Contains(Component.GetComponentId()))
			{
				readCtx.ComponentId = Component.GetComponentId();
				readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

				OnAdded_ReadComponent(readCtx);
			}
		}
	}

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
	virtual void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element)
	{
		RPCReadingContext readCtx;
		readCtx.EntityId = EntityId;
		for (const auto& Component : Element.Components)
		{
			if (ComponentsToReadOnAuthGained.Contains(Component.GetComponentId()))
			{
				readCtx.ComponentId = Component.GetComponentId();
				readCtx.Fields = Schema_GetComponentDataFields(Component.GetUnderlying());

				OnAuthGained_ReadComponent(readCtx);
			}
		}
	}
	virtual void OnAuthGained_ReadComponent(RPCReadingContext& iCtx) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;

protected:
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
};

template <typename PayloadType>
struct TRPCBufferSender : RPCBufferSender
{
	virtual uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, const TArray<PayloadType>& RPCs) = 0;
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
		TArray<PayloadType> RPCs;
		bool bAdded = false;
	};

	TMap<Worker_EntityId, QueueData> Queues;
	TRPCBufferSender<PayloadType>& Sender;
};

class SPATIALGDK_API SpatialRPCService_2
{
public:
	explicit SpatialRPCService_2(const FSubView& InRemoveSubView, const FSubView& InLocalAuthSubView,
								 /*, USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer*/
								 USpatialNetDriver* InNetDriver);

	void AdvanceView();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		FSpatialGDKSpanId SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	void ClearPendingRPCs(Worker_EntityId EntityId);

	struct RPCQueueDescription
	{
		Worker_ComponentSetId Authority;
		TSharedPtr<RPCBufferSender> Sender;
		TSharedPtr<RPCBufferReceiver> Receiver;
		TSharedPtr<RPCQueue> Queue;
	};

	struct RPCReceiverDescription
	{
		Worker_ComponentSetId Authority;
		TSharedPtr<RPCBufferReceiver> Receiver;
	};

	void AddRPCQueue(FName QueueName, RPCQueueDescription&& Desc);
	void AddRPCReceiver(FName ReceiverName, RPCReceiverDescription&& Desc);

private:
	const FSubView* RemoteSubView;
	const FSubView* LocalAuthSubView;

	TMap<FName, RPCQueueDescription> Queues;
	TMap<FName, RPCReceiverDescription> Receivers;
};

} // namespace SpatialGDK
