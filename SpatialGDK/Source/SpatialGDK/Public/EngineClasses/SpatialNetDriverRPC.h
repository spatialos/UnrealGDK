// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/RPCs/RPCService.h"
#include "SpatialView/EntityComponentId.h"
#include "SpatialNetDriverRPC.generated.h"

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

	void ReadFromSchema(Schema_Object* RPCObject);
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
	uint64 Timestamp;
	FSpatialGDKSpanId SpanId;

	void ComputeSpanId(FName Name, SpatialGDK::SpatialEventTracer& Tracer, SpatialGDK::EntityComponentId EntityComponent, uint64 RPCId);
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
		WrappedData(T&& InData)
			: Data(MoveTemp(InData))
		{
		}

		const T& GetData() const { return Data; }

		const FRPCMetaData& GetAdditionalData() const { return MetaData; }

		T Data;
		FName ReceiverName;
		FRPCMetaData MetaData;
	};

	WrappedData MakeWrappedData(Worker_EntityId EntityId, T&& Data, uint64 RPCId)
	{
		WrappedData wrapper(MoveTemp(Data));
		wrapper.ReceiverName = RPCName;
		wrapper.MetaData.Timestamp = FPlatformTime::Cycles64();
		if (EventTracer)
		{
			wrapper.MetaData.ComputeSpanId(RPCName, *EventTracer, SpatialGDK::EntityComponentId(EntityId, ComponentId), RPCId);
		}

		return wrapper;
	}

	FName RPCName;
	Worker_ComponentId ComponentId;
	SpatialGDK::SpatialEventTracer* EventTracer = nullptr;
};

class USpatialLatencyTracer;

/**
 * RPC component for the SpatialNetDriver.
 * It contains the glue between the RPC primitives (queue, sender, receiver) and Unreal Actors,
 * with all the methods and callbacks necessary to send and receive RPCs in UnrealEngine.
 * The base class only contains a receiver for NetMulticast RPCs.
 * Derived class will contain additional RPC components
 */
UCLASS()
class SPATIALGDK_API USpatialNetDriverRPC : public UObject
{
	GENERATED_BODY()
public:
	// Definition for a "standard queue", that is, all queued RPC for sending could have an accompanying SpanId.
	// Makes it easier to define a standard callback for when an outgoing RPC is written to the network.
	using StandardQueue = SpatialGDK::TWrappedRPCQueue<FSpatialGDKSpanId>;

	USpatialNetDriverRPC();
	virtual void Init(USpatialNetDriver* InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
					  const SpatialGDK::FSubView& InActorNonAuthSubView);

	void AdvanceView();
	virtual void ProcessReceivedRPCs();
	virtual void FlushRPCUpdates();
	void FlushRPCQueue(StandardQueue& Queue);
	void FlushRPCQueue(Worker_EntityId, StandardQueue& Queue);
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId);

protected:
	void MakeRingBufferWithACKSender(FName QueueName, ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
									 TUniquePtr<SpatialGDK::RPCBufferSender>& SenderPtr,
									 TUniquePtr<SpatialGDK::TRPCQueue<FRPCPayload, FSpatialGDKSpanId>>& QueuePtr);

	void MakeRingBufferWithACKReceiver(FName QueueName, ERPCType RPCType, Worker_ComponentSetId AuthoritySet,
									   TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>>& ReceiverPtr);

	virtual void GetRPCComponentsOnEntityCreation(const Worker_EntityId EntityId, TArray<FWorkerComponentData>& OutData);

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		TArray<FSpatialGDKSpanId> Spans;
	};

	StandardQueue::SentRPCCallback MakeRPCSentCallback(TArray<UpdateToSend>& OutUpdates);
	SpatialGDK::RPCCallbacks::DataWritten MakeDataWriteCallback(TArray<FWorkerComponentData>& OutArray) const;
	SpatialGDK::RPCCallbacks::UpdateWritten MakeUpdateWriteCallback(TArray<UpdateToSend>& OutUpdates) const;

	void FlushUpdates(TArray<UpdateToSend>&);
	bool CanExtractRPC(Worker_EntityId);
	bool CanExtractRPCOnServer(Worker_EntityId);
	bool ApplyRPC(Worker_EntityId, SpatialGDK::ReceivedRPC, const FRPCMetaData& MetaData);

	USpatialNetDriver* NetDriver;

	USpatialLatencyTracer* LatencyTracer;
	SpatialGDK::SpatialEventTracer* EventTracer;

	TUniquePtr<SpatialGDK::RPCService> RPCService;
	// TUniquePtr<SpatialGDK::TRPCBufferReceiver<FRPCPayload, TimestampAndETWrapper>> NetMulticastReceiver;
};

/**
 * Server side of the RPC component.
 * Contains client, cross server and multicast senders
 * Contains server and cross server receivers
 * Able to collect initial RPC data for entities to create
 */
UCLASS()
class SPATIALGDK_API USpatialNetDriverServerRPC : public USpatialNetDriverRPC
{
	GENERATED_BODY()
public:
	USpatialNetDriverServerRPC();
	void Init(USpatialNetDriver* InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
			  const SpatialGDK::FSubView& InActorNonAuthSubView) override;

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
UCLASS()
class SPATIALGDK_API USpatialNetDriverClientRPC : public USpatialNetDriverRPC
{
	GENERATED_BODY()
public:
	USpatialNetDriverClientRPC();
	void Init(USpatialNetDriver* InNetDriver, const SpatialGDK::FSubView& InActorAuthSubView,
			  const SpatialGDK::FSubView& InActorNonAuthSubView) override;

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
