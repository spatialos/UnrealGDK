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

struct FRPCPayload
{
	uint32 Offset;
	uint32 Index;
	TArray<uint8> PayloadData;

	void ReadFromSchema(Schema_Object* RPCObject);
	void WriteToSchema(Schema_Object* RPCObject) const;
};

struct FRPCCommandPayload : FRPCPayload
{
	uint64 UUID;
};

struct FRPCCrossServerPayload : FRPCPayload
{
	uint64 UUID;
	Worker_EntityId Counterpart;
};

struct FRPCMetaData
{
	uint64 Timestamp;
	FSpatialGDKSpanId SpanId;

	void ComputeSpanId(FName Name, SpatialGDK::SpatialEventTracer& Tracer, SpatialGDK::EntityComponentId EntityComponent, uint64 RPCId);
};

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

UCLASS()
class SPATIALGDK_API USpatialNetDriverRPC : public UObject
{
	GENERATED_BODY()
public:
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
