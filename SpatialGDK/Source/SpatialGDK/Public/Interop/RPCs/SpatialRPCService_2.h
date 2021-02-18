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
		EntityWrite& operator =(const EntityWrite&) = delete;
		EntityWrite& operator =(EntityWrite&&) = delete;

		EntityWrite(EntityWrite&& Write)
			: Ctx(Write.Ctx)
			, EntityId(Write.EntityId)
			, ComponentId(Write.ComponentId)
		{
			Write.bActiveWriter = false;
		}

		~EntityWrite()
		{
			if (bActiveWriter)
			{
				switch (Ctx.Kind)
				{
				case DataKind::Generic:
					break;
				case DataKind::Data:
					if (Ctx.DataWrittenCallback)
					{
						Ctx.DataWrittenCallback(EntityId, ComponentId, Data);
					}
					break;
				case DataKind::Update:
					if (Ctx.UpdateWrittenCallback)
					{
						Ctx.UpdateWrittenCallback(EntityId, ComponentId, Update);
					}
					break;
				case DataKind::CommandRequest:
					if (Ctx.RequestWrittenCallback)
					{
						Ctx.RequestWrittenCallback(EntityId, Request);
					}
					break;
				case DataKind::CommandResponse:
					if (Ctx.ResponseWrittenCallback)
					{
						Ctx.ResponseWrittenCallback(EntityId, Response);
					}
					break;
				}
			}
		}
		
		const Worker_EntityId EntityId;
		const Worker_ComponentId ComponentId;

		Schema_ComponentUpdate* GetComponentUpdateToWrite()
		{
			check(Ctx.Kind == DataKind::Update);
			GetFieldsToWrite();
			return Update;
		}

		Schema_Object* GetFieldsToWrite()
		{
			if (Fields == nullptr)
			{
				switch (Ctx.Kind)
				{
				case DataKind::Generic:
					GenData = Schema_CreateGenericData();
					Fields = Schema_GetGenericDataObject(GenData);
					break;
				case DataKind::Data:
					Data = Schema_CreateComponentData();
					Fields = Schema_GetComponentDataFields(Data);
					break;
				case DataKind::Update:
					Update = Schema_CreateComponentUpdate();
					Fields = Schema_GetComponentUpdateFields(Update);
					break;
				case DataKind::CommandRequest:
					Request = Schema_CreateCommandRequest();
					Fields = Schema_GetCommandRequestObject(Request);
					break;
				case DataKind::CommandResponse:
					Response = Schema_CreateCommandResponse();
					Fields = Schema_GetCommandResponseObject(Response);
					break;
				}
			}
			return Fields;
		}

		void RPCWritten(uint32 RPCId)
		{
			if (Ctx.RPCWrittenCallback)
			{
				Ctx.RPCWrittenCallback(EntityId, ComponentId, RPCId);
			}
		}

	private:
		union
		{
			Schema_GenericData* GenData;
			Schema_ComponentData* Data;
			Schema_ComponentUpdate* Update;
			Schema_CommandRequest* Request;
			Schema_CommandResponse* Response;
		};

		EntityWrite(RPCWritingContext& InCtx, Worker_EntityId InEntityId, Worker_ComponentId InComponentID)
			: Ctx(InCtx)
			, EntityId(InEntityId)
			, ComponentId(InComponentID)
		{
			
		}
		friend RPCWritingContext;
		RPCWritingContext& Ctx;
		Schema_Object* Fields = nullptr;
		bool bActiveWriter = true;
	};

	EntityWrite WriteTo(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
	{
		return EntityWrite(*this, EntityId, ComponentId);
	}

	RPCWritingContext(DataKind InKind)
		: Kind(InKind)
	{
	
	}

	TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentData*)> DataWrittenCallback;
	TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentUpdate*)> UpdateWrittenCallback;
	TFunction<void(Worker_EntityId, Schema_CommandRequest*)> RequestWrittenCallback;
	TFunction<void(Worker_EntityId, Schema_CommandResponse*)> ResponseWrittenCallback;
	
	TFunction<void(Worker_EntityId, Worker_ComponentId, uint32)> RPCWrittenCallback;
	

protected:
	DataKind Kind;
};

#if 0

struct RPCPayloadIterator
{
	virtual int32 Num() = 0;
	virtual void Reset() = 0;
	virtual RPCPayload& operator*() = 0;
	virtual RPCPayloadIterator& operator++() = 0;
	virtual explicit operator bool() = 0;
};

template <typename Payload>
struct TRPCPayloadIterator
{
	TRPCPayloadIterator(Payload* InBegin, Payload* InEnd)
		: Begin(InBegin)
		, End(InEnd)
	{
		Current = Begin;
	}

	int32 Num()
	{
		return End - Begin;
	}

	void Reset()
	{
		Current = Begin;
	}

	Payload& operator*() override
	{
		return *Current;
	}

	RPCPayloadIterator& operator++() override
	{
		Current++;
	}

	explicit operator bool() override
	{
		return Begin != End;
	}

	Payload* Begin;
	Payload* Current;
	Payload* End;
};

#endif

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

	//virtual uint32 Write(RPCWritingContext& Ctx, RPCPayloadIterator& RPCs) = 0;

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
	//virtual void Push(Worker_EntityId EntityId, RPCPayload* Data) = 0;
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
	{}

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
	//static constexpr uint32 ClientReliable = 0;
	//static constexpr uint32 ClientUnreliable = 1;
	//static constexpr uint32 ServerReliable = 2;
	//static constexpr uint32 ServerUnreliable = 3;
	//static constexpr uint32 Multicast = 4;
	//static constexpr uint32 DedicatedMovementQueue = 5;

	explicit SpatialRPCService_2(const FSubView& InRemoveSubView,
								 const FSubView& InLocalAuthSubView,
								 /*, USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer*/
								 USpatialNetDriver* InNetDriver);

	void AdvanceView();
	void ProcessChanges(const float NetDriverTime);

	void ProcessIncomingRPCs();

	//void ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload,
	//							   TOptional<uint64> RPCIdForLinearEventTrace);

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		FSpatialGDKSpanId SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	void ClearPendingRPCs(Worker_EntityId EntityId);

	//template <typename T>
	//void PushRPC(uint32 QueueName, Worker_EntityId Entity, const T& Data)
	//{
	//	//Schema_GenericData* Data = Schema_CreateGenericData();
	//	//Schema_Object* Object = Schema_GetGenericDataObject(Data);
	//	//Data.WriteToSchema(Object);
	//
	//	//TArray<uint8> SerializedObject(Schema_GetWriteBufferLength(Data));
	//	//Schema_SerializeToBuffer(Object, SerializedObject.GetData(), SerializedObject.Num());
	//	//
	//	//PushRPC(QueueName, Entity, MoveTemp(SerializedObject));
	//	//
	//	//Schema_DestroyGenericData(Data);
	//}
	//
	//void PushRPC(uint32 QueueName, Worker_EntityId Entity, void* Data);

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
	FRPCErrorInfo ApplyRPC(const FPendingRPCParams& Params);
	FRPCErrorInfo ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams);

	const FSubView* RemoteSubView;
	const FSubView* LocalAuthSubView;

	FRPCContainer IncomingRPCs{ ERPCQueueType::Receive };

	TMap<FName, RPCQueueDescription> Queues;
	TMap<FName, RPCReceiverDescription> Receivers;
};

} // namespace SpatialGDK
