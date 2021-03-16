// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/RPCs/RPCService.h"
#include "SpatialView/EntityComponentId.h"

#include <atomic>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetDriverRPC, Log, All);

namespace SpatialGDK
{
class SpatialEventTracer;
}

/**
 * Base RPC type
 * Different from RPCPayload to have stricter definition of outgoing RPC data
 * and to see if the templated version of the queue/sender/receiver is viable.
 */
struct FRPCPayload
{
	uint32 Offset;
	uint32 Index;
	TArray<uint8> PayloadData;

	void ReadFromSchema(const Schema_Object* RPCObject);
	void WriteToSchema(Schema_Object* RPCObject) const;
};

/**
 * Payload version for commands, adding a unique Id to try to prevent double-execution
 * when double sending happens.
 * NOTE : to have a uniform cross-server RPC API, the queue and sender would use a FRPCCrossServerPayload template parameter,
 * but the sender would write a FRPCCommandPayload and the receiver would use a FRPCCommandPayload template parameter
 */
struct FRPCCommandPayload : FRPCPayload
{
	uint64 UUID;
};

/**
 * Payload for cross-server RPCs.
 * Depending on the component this payload is written to, Counterpart would indicate either the sender or the receiver.
 * This is mainly intended to be used with the RoutingWorker implementation of cross-server RPCs,
 * which is responsible for swapping the entity the RPC is written on and the counterpart.
 */
struct FRPCCrossServerPayload : FRPCPayload
{
	uint64 UUID;
	Worker_EntityId Counterpart;
};

/**
 * Additional data accompanying a received RPC.
 */
struct FRPCMetaData
{
	FName RPCName;
	uint64 Timestamp;
	FSpatialGDKSpanId SpanId;

	void ComputeSpanId(SpatialGDK::SpatialEventTracer& Tracer, SpatialGDK::EntityComponentId EntityComponent, uint64 RPCId);
};

/**
 * Wrapper used to add Timestamp and event tracing information to a received RPC.
 */
template <typename T>
struct TimestampAndETWrapper
{
	TimestampAndETWrapper(FName InRPCName, Worker_ComponentId InComponentId, SpatialGDK::SpatialEventTracer* InEventTracer = nullptr)
		: RPCName(InRPCName)
		, ComponentId(InComponentId)
		, EventTracer(InEventTracer)
	{
	}

	using AdditionalData = FRPCMetaData;
	struct WrappedData
	{
		WrappedData() = default;
		WrappedData(T&& InData, FName RPCName)
			: Data(MoveTemp(InData))
		{
			MetaData.RPCName = RPCName;
			MetaData.Timestamp = FPlatformTime::Cycles64();
		}

		const T& GetData() const { return Data; }

		const FRPCMetaData& GetAdditionalData() const { return MetaData; }

		T Data;
		FRPCMetaData MetaData;
	};

	WrappedData MakeWrappedData(Worker_EntityId EntityId, T&& Data, uint64 RPCId)
	{
		WrappedData Wrapper(MoveTemp(Data), RPCName);
		if (EventTracer != nullptr)
		{
			Wrapper.MetaData.ComputeSpanId(RPCName, *EventTracer, SpatialGDK::EntityComponentId(EntityId, ComponentId), RPCId);
		}

		return Wrapper;
	}

	FName RPCName;
	Worker_ComponentId ComponentId;
	SpatialGDK::SpatialEventTracer* EventTracer = nullptr;
};

/**
 * RPC component for the SpatialNetDriver.
 * It contains the glue between the RPC primitives (queue, sender, receiver) and Unreal Actors,
 * with all the methods and callbacks necessary to send and receive RPCs in UnrealEngine.
 * The base class only contains a receiver for NetMulticast RPCs.
 * Derived class will contain additional RPC components
 */
class SPATIALGDK_API FSpatialNetDriverRPC : public UObject
{
public:
	// Definition for a "standard queue", that is, all queued RPC for sending could have an accompanying SpanId.
	// Makes it easier to define a standard callback for when an outgoing RPC is written to the network.
	using StandardQueue = SpatialGDK::TWrappedRPCQueue<FSpatialGDKSpanId>;

	FSpatialNetDriverRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
						 const SpatialGDK::FSubView& InActorNonAuthSubView);

	void AdvanceView();
	virtual void ProcessReceivedRPCs();
	virtual void FlushRPCUpdates();
	void FlushRPCQueue(StandardQueue& Queue);
	void FlushRPCQueueForEntity(Worker_EntityId, StandardQueue& Queue);
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId);

protected:
	void MakeRingBufferWithACKSender(ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
									 TUniquePtr<SpatialGDK::RPCBufferSender>& SenderPtr,
									 TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>>& QueuePtr);

	void MakeRingBufferWithACKReceiver(ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
									   TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>>& ReceiverPtr);

	virtual void GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData);

	struct UpdateToSend : FNoHeapAllocation
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		TArray<FSpatialGDKSpanId> Spans;
	};

	StandardQueue::SentRPCCallback MakeRPCSentCallback();
	SpatialGDK::RPCCallbacks::DataWritten MakeDataWriteCallback(TArray<FWorkerComponentData>& OutArray) const;
	SpatialGDK::RPCCallbacks::UpdateWritten MakeUpdateWriteCallback();

	static void OnRPCSent(SpatialGDK::SpatialEventTracer& EventTracer, TArray<UpdateToSend>& OutUpdates, FName Name,
						  Worker_EntityId EntityId, Worker_ComponentId ComponentId, uint64 RPCId, const FSpatialGDKSpanId& SpanId);
	static void OnDataWritten(TArray<FWorkerComponentData>& OutArray, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
							  Schema_ComponentData* InData);
	static void OnUpdateWritten(TArray<UpdateToSend>& OutUpdates, Worker_EntityId EntityId, Worker_ComponentId ComponentId,
								Schema_ComponentUpdate* InUpdate);

	bool CanExtractRPC(Worker_EntityId EntityId) const;
	bool CanExtractRPCOnServer(Worker_EntityId EntityId) const;
	bool ApplyRPC(Worker_EntityId EntityId, SpatialGDK::ReceivedRPC ReceivedCallback, const FRPCMetaData& MetaData) const;

	USpatialNetDriver& NetDriver;
	SpatialGDK::SpatialEventTracer* EventTracer = nullptr;
	TUniquePtr<SpatialGDK::RPCService> RPCService;
	
	// Caching the array of updates to send, to avoid a reallocation each frame.
	TArray<UpdateToSend> UpdateToSend_Cache;
	std::atomic<bool> bUpdateCacheInUse;
	struct RAIIUpdateContext;

	// TODO UNR-5038
	// TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> NetMulticastReceiver;
};

/**
 * Server side of the RPC component.
 * Contains client, cross server and multicast senders
 * Contains server and cross server receivers
 * Able to collect initial RPC data for entities to create
 */
class SPATIALGDK_API FSpatialNetDriverServerRPC : public FSpatialNetDriverRPC
{
public:
	FSpatialNetDriverServerRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
							   const SpatialGDK::FSubView& InActorNonAuthSubView);

	void ProcessReceivedRPCs() override;
	void FlushRPCUpdates() override;

	TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>> ClientReliableQueue;
	TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>> ClientUnreliableQueue;
	TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>> NetMulticastQueue;

protected:
	void GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData) override;

	TUniquePtr<SpatialGDK::RPCBufferSender> ClientReliableSender;
	TUniquePtr<SpatialGDK::RPCBufferSender> ClientUnreliableSender;
	TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> ServerReliableReceiver;
	TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> ServerUnreliableReceiver;
};

/**
 * Client side of the RPC component.
 * It contains server senders, and client receivers.
 */
class SPATIALGDK_API FSpatialNetDriverClientRPC : public FSpatialNetDriverRPC
{
public:
	FSpatialNetDriverClientRPC(USpatialNetDriver& InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
							   const SpatialGDK::FSubView& InActorNonAuthSubView);

	void ProcessReceivedRPCs() override;
	void FlushRPCUpdates() override;

	TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>> ServerReliableQueue;
	TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>> ServerUnreliableQueue;

protected:
	void GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData) override;

	TUniquePtr<SpatialGDK::RPCBufferSender> ServerReliableSender;
	TUniquePtr<SpatialGDK::RPCBufferSender> ServerUnreliableSender;
	TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> ClientReliableReceiver;
	TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> ClientUnreliableReceiver;
};
