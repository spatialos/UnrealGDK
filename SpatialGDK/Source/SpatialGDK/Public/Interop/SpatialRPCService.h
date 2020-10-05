// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/RPCPayload.h"
#include "SpatialView/EntityComponentId.h"
#include "Utils/CrossServerUtils.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService, Log, All);

class SpatialOSWorkerInterface;

class USpatialLatencyTracer;
class USpatialStaticComponentView;
struct RPCRingBuffer;

DECLARE_DELEGATE_RetVal_FiveParams(bool, ExtractRPCDelegate, Worker_EntityId, const FUnrealObjectRef&, ERPCType,
								   const SpatialGDK::RPCPayload&, uint64);

namespace SpatialGDK
{
struct CrossServerEndpointACK;

struct EntityRPCType
{
	EntityRPCType(Worker_EntityId EntityId, ERPCType Type)
		: EntityId(EntityId)
		, Type(Type)
	{
	}

	Worker_EntityId EntityId;
	ERPCType Type;

	friend bool operator==(const EntityRPCType& Lhs, const EntityRPCType& Rhs)
	{
		return Lhs.EntityId == Rhs.EntityId && Lhs.Type == Rhs.Type;
	}

	friend uint32 GetTypeHash(EntityRPCType Value)
	{
		return HashCombine(::GetTypeHash(static_cast<int64>(Value.EntityId)), ::GetTypeHash(static_cast<uint32>(Value.Type)));
	}
};

enum class EPushRPCResult : uint8
{
	Success,

	QueueOverflowed,
	DropOverflowed,
	HasAckAuthority,
	NoRingBufferAuthority,
	EntityBeingCreated
};

class SPATIALGDK_API SpatialRPCService
{
public:
	SpatialRPCService(ExtractRPCDelegate ExtractRPCCallback, const USpatialStaticComponentView* View,
					  USpatialLatencyTracer* SpatialLatencyTracer);

	EPushRPCResult PushRPC(Worker_EntityId EntityId, const FUnrealObjectRef& Counterpart, ERPCType Type, RPCPayload Payload,
						   bool bCreatedEntity);
	void PushOverflowedRPCs();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);
	void CheckLocalTargets(Worker_EntityId SenderId);
	void InspectStaleEntities();
	void HandleTimeout(Worker_EntityId SenderId, SpatialOSWorkerInterface* Sender);
	bool OnEntityRequestResponse(Worker_EntityId SenderId, Worker_RequestId Request, bool bEntityExists);
	void OnEntityRemoved(Worker_EntityId SenderId, Worker_EntityId Entity);

	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCsForEntity(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// Will also store acked IDs locally.
	void IncrementAckedRPCID(Worker_EntityId EntityId, ERPCType Type);

	void OnCheckoutMulticastRPCComponentOnEntity(Worker_EntityId EntityId);
	void OnRemoveMulticastRPCComponentForEntity(Worker_EntityId EntityId);

	void OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void OnEndpointAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	void WriteCrossServerACKFor(Worker_EntityId Receiver, Worker_EntityId Sender, uint64 RPCId, uint64 SenderRev, ERPCType Type);
	void UpdateMergedACKs(Worker_EntityId RemoteReceiver);

private:
	// For now, we should drop overflowed RPCs when entity crosses the boundary.
	// When locking works as intended, we should re-evaluate how this will work (drop after some time?).
	void ClearOverflowedRPCs(Worker_EntityId EntityId);

	EPushRPCResult PushRPCInternal(Worker_EntityId EntityId, const FUnrealObjectRef& Counterpart, ERPCType Type, RPCPayload&& Payload,
								   bool bCreatedEntity);

	void ExtractRPCsForType(Worker_EntityId EntityId, ERPCType Type);
	void ExtractCrossServerRPCsForType(Worker_EntityId EntityId, ERPCType Type);

	void AddOverflowedRPC(EntityRPCType EntityType, RPCPayload&& Payload);

	uint64 GetAckFromView(Worker_EntityId EntityId, ERPCType Type);
	const RPCRingBuffer& GetBufferFromView(Worker_EntityId EntityId, ERPCType Type);
	bool HasCheckedOutBuffer(Worker_EntityId EntityId, ERPCType Type);

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair);
	Schema_ComponentData* GetOrCreateComponentData(EntityComponentId EntityComponentIdPair);

private:
	void CleanupACKsFor(Worker_EntityId Sender, ERPCType Type);
	CrossServerEndpointACK* GetACKEndpoint(Worker_EntityId EntityId, ERPCType Type);

	ExtractRPCDelegate ExtractRPCCallback;
	const USpatialStaticComponentView* View;
	USpatialLatencyTracer* SpatialLatencyTracer;

	// This is local, not written into schema.
	TMap<Worker_EntityId_Key, uint64> LastSeenMulticastRPCIds;
	TMap<EntityRPCType, uint64> LastSeenRPCIds;

	// Stored here for things we have authority over.
	TMap<EntityRPCType, uint64> LastAckedRPCIds;
	TMap<EntityRPCType, uint64> LastSentRPCIds;

	TMap<EntityComponentId, Schema_ComponentData*> PendingRPCsOnEntityCreation;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;
	TMap<EntityRPCType, TArray<RPCPayload>> OverflowedRPCs;

	TMap<Worker_EntityId_Key, CrossServer::SenderState> ActorSenderState;

	// For receiver
	// Contains the number of available slots for acks for the given receiver.
	TMap<Worker_EntityId_Key, CrossServer::SlotAlloc> ACKAllocMap;

	struct ReceiverState
	{
		CrossServer::RPCSchedule Schedule;
		struct Item
		{
			uint32 Slot;
			CrossServer::ACKSlot ACKSlot;
		};
		TMap<CrossServer::RPCKey, Item> Slots;
	};

	// Map from receiving endpoint to ack slots.
	TMap<Worker_EntityId_Key, ReceiverState> ReceiverMap;

	TSet<Worker_EntityId_Key> StaleEntities;

#if TRACE_LIB_ACTIVE
	void ProcessResultToLatencyTrace(const EPushRPCResult Result, const TraceKey Trace);
	TMap<EntityComponentId, TraceKey> PendingTraces;
#endif

	bool bShouldCheckSentRPCForLocalTarget = false;
};

} // namespace SpatialGDK
