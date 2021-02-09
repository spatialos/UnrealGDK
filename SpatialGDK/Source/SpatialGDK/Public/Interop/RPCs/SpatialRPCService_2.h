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
class RPCWriter
{
public:
	virtual ~RPCWriter() = default;
	virtual Worker_ComponentId GetComponentToWriteTo() const = 0;
	virtual uint32 GetFreeSlots(Worker_EntityId) const = 0;
	// Distinction might be more relevant for readers
	virtual void OnUpdate(Worker_EntityId, const ComponentChange& Update) = 0;
	virtual void OnCompleteUpdate(Worker_EntityId, const ComponentChange& Update) = 0;
	virtual void OnAuthGained(Worker_EntityId, EntityViewElement const& Element) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;
	virtual uint32 Write(Worker_EntityId EntityId, Schema_Object* ComponentObject, const uint8* RPCObjectBytes, uint32 RPCObjectSize) = 0;
};

struct RPCQueue
{
	using FlushCallback = TFunction<void(Worker_EntityId, const TArray<uint8>*, uint32)>;

	virtual void Init(RPCWriter* Writer) = 0;
	virtual void Push(Worker_EntityId EntityId, TArray<uint8> Data) = 0;
	virtual void FlushAll(FlushCallback&) = 0;
	virtual void Flush(Worker_EntityId EntityId, FlushCallback&) = 0;
	virtual void OnAuthGained(Worker_EntityId, EntityViewElement const& Element) = 0;
	virtual void OnAuthLost(Worker_EntityId) = 0;
};

class SPATIALGDK_API SpatialRPCService_2
{
public:
	static constexpr uint32 ClientReliable = 0;
	static constexpr uint32 ClientUnreliable = 1;
	static constexpr uint32 ServerReliable = 2;
	static constexpr uint32 ServerUnreliable = 3;
	static constexpr uint32 Multicast = 4;
	static constexpr uint32 DedicatedMovementQueue = 5;

	explicit SpatialRPCService_2(const FSubView& InActorAuthSubView,
								 const FSubView& InActorNonAuthSubView
								 /*, USpatialLatencyTracer* InSpatialLatencyTracer, SpatialEventTracer* InEventTracer*/
								 ,
								 USpatialNetDriver* InNetDriver);

	void AdvanceView();
	void ProcessChanges(const float NetDriverTime);

	void ProcessIncomingRPCs();

	void ProcessOrQueueIncomingRPC(const FUnrealObjectRef& InTargetObjectRef, RPCPayload InPayload,
								   TOptional<uint64> RPCIdForLinearEventTrace);

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
		FSpatialGDKSpanId SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	void ClearPendingRPCs(Worker_EntityId EntityId);

	template <typename T>
	void PushRPC(uint32 QueueName, Worker_EntityId Entity, const T& Data)
	{
		Schema_GenericData* Data = Schema_CreateGenericData();
		Schema_Object* Object = Schema_GetGenericDataObject(Data);
		Data.WriteToSchema(Object);

		TArray<uint8> SerializedObject(Schema_GetWriteBufferLength(Data));
		Schema_SerializeToBuffer(Object, SerializedObject.GetData(), SerializedObject.Num());

		PushRPC(QueueName, Entity, MoveTemp(SerializedObject));

		Schema_DestroyGenericData(Data);
	}

	void PushRPC(uint32 QueueName, Worker_EntityId Entity, TArray<uint8> Data);

	struct RPCDescription
	{
		Worker_ComponentSetId Authority;
		TUniquePtr<RPCWriter> Writer;
		TUniquePtr<RPCQueue> Queue;
	};

	void AddRPCQueue(uint32 QueueName, Worker_ComponentSetId Authority, TUniquePtr<RPCWriter> Writer, TUniquePtr<RPCQueue> Queue);

private:
	FRPCErrorInfo ApplyRPC(const FPendingRPCParams& Params);
	FRPCErrorInfo ApplyRPCInternal(UObject* TargetObject, UFunction* Function, const FPendingRPCParams& PendingRPCParams);

	const FSubView* AuthSubView;
	const FSubView* ActorSubView;

	FRPCContainer IncomingRPCs{ ERPCQueueType::Receive };

	TMap<uint32, RPCDescription> Queues;
};

} // namespace SpatialGDK
