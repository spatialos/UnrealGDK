// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/EntityView.h"
#include "Utils/ObjectAllocUtils.h"

namespace SpatialGDK
{
struct ReceivedRPC : FNoHeapAllocation
{
	ReceivedRPC(uint32 InOffset, uint32 InIndex, TArrayView<const uint8> InPayloadData)
		: Offset(InOffset)
		, Index(InIndex)
		, PayloadData(InPayloadData)
	{
	}
	const uint32 Offset;
	const uint32 Index;
	const TArrayView<const uint8> PayloadData;
};

namespace RPCCallbacks
{
using DataWritten = TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentData*)>;
using UpdateWritten = TFunction<void(Worker_EntityId, Worker_ComponentId, Schema_ComponentUpdate*)>;
using RequestWritten = TFunction<void(Worker_EntityId, Schema_CommandRequest*)>;
using ResponseWritten = TFunction<void(Worker_EntityId, Schema_CommandResponse*)>;
using RPCWritten = TFunction<void(Worker_ComponentId, uint64)>;

using CanExtractRPCs = TFunction<bool(Worker_EntityId)>;
} // namespace RPCCallbacks

/**
 * Structure encapsulating a read operation
 */
struct RPCReadingContext : FStackOnly
{
	FName ReaderName;
	Worker_EntityId EntityId;
	Worker_ComponentId ComponentId;

	bool IsUpdate() const { return Update != nullptr; }

	Schema_ComponentUpdate* Update = nullptr;
	Schema_Object* Fields = nullptr;
};

/**
 * Structure encapsulating a write operation.
 * It serves as a factory for EntityWrites which encapsulate writes to a given entity/component pair
 */
struct RPCWritingContext : FStackOnly
{
	enum class DataKind
	{
		Generic,
		ComponentData,
		ComponentUpdate,
		CommandRequest,
		CommandResponse
	};

	RPCWritingContext(FName, RPCCallbacks::DataWritten DataWrittenCallback);
	RPCWritingContext(FName, RPCCallbacks::UpdateWritten UpdateWrittenCallback);
	RPCWritingContext(FName, RPCCallbacks::RequestWritten RequestWrittenCallback);
	RPCWritingContext(FName, RPCCallbacks::ResponseWritten ResponseWrittenCallback);

	/**
	 * RAII object to encapsulate writes to an entity/component couple.
	 * It makes sure that the appropriate callback is executed when the write operation is done
	 */
	class EntityWrite : FStackOnly
	{
	public:
		EntityWrite(const EntityWrite&) = delete;
		EntityWrite& operator=(const EntityWrite&) = delete;
		EntityWrite& operator=(EntityWrite&&) = delete;

		EntityWrite(EntityWrite&& Write);
		~EntityWrite();

		Schema_ComponentUpdate* GetComponentUpdateToWrite();
		Schema_Object* GetFieldsToWrite();

		// Must be called by writers to allow any per-RPC operation to take place.
		// The RPCId parameter is intended to be unique in the EntityId/ComponentId context.
		// void RPCWritten(uint64 RPCId);

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

		// indicates if this object has been moved from, and if callback should be ran on destruction.
		bool bActiveWriter = true;
	};

	EntityWrite WriteTo(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

protected:
	RPCCallbacks::DataWritten DataWrittenCallback;
	RPCCallbacks::UpdateWritten UpdateWrittenCallback;
	RPCCallbacks::RequestWritten RequestWrittenCallback;
	RPCCallbacks::ResponseWritten ResponseWrittenCallback;

	FName QueueName;
	const DataKind Kind;

	bool bWriterOpened = false;
};

/**
 * Class responsible for managing the sending side of a given RPC type
 * It will operate on the locally authoritative view of the actors.
 */
class RPCBufferSender
{
public:
	virtual ~RPCBufferSender() = default;

	virtual void OnUpdate(const RPCReadingContext& iCtx) = 0;
	virtual void OnAuthGained(Worker_EntityId EntityId, EntityViewElement const& Element);
	virtual void OnAuthGained_ReadComponent(const RPCReadingContext& iCtx) = 0;
	virtual void OnAuthLost(Worker_EntityId EntityId) = 0;

	const TSet<Worker_ComponentId>& GetComponentsToReadOnUpdate() const { return ComponentsToReadOnUpdate; }

protected:
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
	TSet<Worker_ComponentId> ComponentsToReadOnUpdate;
};

/**
 * Class responsible for managing the receiving side of a given RPC type
 * It will operate on the actor view, on actors it may or may not have local authority on.
 */
class RPCBufferReceiver
{
public:
	virtual ~RPCBufferReceiver() = default;

	virtual void OnAdded(FName ReceiverName, Worker_EntityId EntityId, EntityViewElement const& Element);
	virtual void OnAdded_ReadComponent(const RPCReadingContext& Ctx) = 0;
	virtual void OnRemoved(Worker_EntityId EntityId) = 0;
	virtual void OnUpdate(const RPCReadingContext& iCtx) = 0;
	virtual void FlushUpdates(RPCWritingContext& Ctx) = 0;

	const TSet<Worker_ComponentId>& GetComponentsToRead() const { return ComponentsToRead; }

protected:
	TSet<Worker_ComponentId> ComponentsToRead;
};

struct RPCEmptyData
{
};

template <typename T>
struct NullReceiveWrapper
{
	using AdditionalData = RPCEmptyData;
	struct WrappedData
	{
		WrappedData(T&& InData)
			: Data(MoveTemp(InData))
		{
		}

		const AdditionalData& GetAdditionalData() const
		{
			static EmptyData s_Dummy;
			return s_Dummy;
		}

		const T& GetData() const { return Data; }

		T Data;
	};

	WrappedData MakeWrappedData(Worker_EntityId EntityId, T&& Data, uint64 RPCId) { return MoveTemp(Data); }
};

template <typename T, template <typename> class PayloadWrapper = NullReceiveWrapper>
class TRPCBufferReceiver : public RPCBufferReceiver
{
public:
	TRPCBufferReceiver(PayloadWrapper<T>&& InWrapper = PayloadWrapper<T>())
		: Wrapper(MoveTemp(InWrapper))
	{
	}

	using ProcessRPC = TFunction<bool(Worker_EntityId, ReceivedRPC, typename PayloadWrapper<T>::AdditionalData const&)>;
	virtual void ExtractReceivedRPCs(const RPCCallbacks::CanExtractRPCs&, const ProcessRPC&) = 0;

	void QueueReceivedRPC(Worker_EntityId EntityId, T&& Data, uint64 RPCId)
	{
		auto& RPCs = ReceivedRPCs.FindOrAdd(EntityId);
		RPCs.Emplace(Wrapper.MakeWrappedData(EntityId, MoveTemp(Data), RPCId));
	}

protected:
	TMap<Worker_EntityId_Key, TArray<typename PayloadWrapper<T>::WrappedData>> ReceivedRPCs;
	PayloadWrapper<T> Wrapper;
};

/**
 * Class responsible for the local queuing behaviour when sending.
 * Local queuing is mostly useful when we are in the process of creating an entity and
 * cannot send the RPCs right away, and when the ring buffer sender does not have capacity to send the RPCs.
 */
struct RPCQueue
{
	virtual ~RPCQueue() = default;
	virtual void OnAuthGained(Worker_EntityId EntityId, const EntityViewElement& Element);
	virtual void OnAuthGained_ReadComponent(const RPCReadingContext& Ctx) = 0;
	virtual void OnAuthLost(Worker_EntityId EntityId) = 0;

	const FName Name;

protected:
	RPCQueue(FName InName)
		: Name(InName)
	{
	}
	TSet<Worker_ComponentId> ComponentsToReadOnAuthGained;
};

/**
 * Specialization of a buffer sender for a given payload type
 * It is paired with a matching queue.
 */
template <typename PayloadType>
struct TRPCBufferSender : RPCBufferSender
{
	virtual uint32 Write(RPCWritingContext& Ctx, Worker_EntityId EntityId, TArrayView<const PayloadType> RPC,
						 const RPCCallbacks::RPCWritten& WrittenCallback) = 0;
};

template <typename AdditionalSendingData>
struct TWrappedRPCQueue : public RPCQueue
{
	using SentRPCCallback = TFunction<void(FName, Worker_EntityId, Worker_ComponentId, uint64, const AdditionalSendingData&)>;

	virtual void FlushAll(RPCWritingContext& Ctx, const SentRPCCallback&SentCallback) = 0;
	virtual void Flush(Worker_EntityId EntityId, RPCWritingContext& Ctx, const SentRPCCallback&, bool bIgnoreAdded = false) = 0;

protected:
	TWrappedRPCQueue(FName InName)
		: RPCQueue(InName)
	{
	}
};

/**
 * Specialization of a sender queue for a given payload type
 * It is paired with a matching sender.
 */
template <typename PayloadType, typename AdditionalSendingData = RPCEmptyData>
struct TRPCQueue : TWrappedRPCQueue<AdditionalSendingData>
{
	TRPCQueue(FName InName, TRPCBufferSender<PayloadType>& InSender)
		: TWrappedRPCQueue<AdditionalSendingData>(InName)
		, Sender(InSender)
	{
	}

	virtual void Push(Worker_EntityId EntityId, PayloadType&& Payload, AdditionalSendingData&& Add = AdditionalSendingData())
	{
		auto& Queue = Queues.FindOrAdd(EntityId);
		Queue.RPCs.Emplace(MoveTemp(Payload));
		Queue.AddData.Emplace(MoveTemp(Add));
	}

	void OnAuthGained(Worker_EntityId EntityId, const EntityViewElement& Element) override
	{
		RPCQueue::OnAuthGained(EntityId, Element);
		Queues.FindOrAdd(EntityId).bAdded = true;
	}

	struct QueueData
	{
		// Most RPCs are flushed right after queuing them,
		// so a small array optimization looks useful in general.
		TArray<PayloadType, TInlineAllocator<1>> RPCs;
		TArray<AdditionalSendingData, TInlineAllocator<1>> AddData;
		bool bAdded = false;
	};

	TMap<Worker_EntityId_Key, QueueData> Queues;
	TRPCBufferSender<PayloadType>& Sender;
};

} // namespace SpatialGDK
