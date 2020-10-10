// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Interop/EntityRPCType.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/EntityComponentId.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService, Log, All);

class USpatialLatencyTracer;
class USpatialStaticComponentView;
struct RPCRingBuffer;

DECLARE_DELEGATE_RetVal_ThreeParams(bool, ExtractRPCDelegate, FEntityId, ERPCType, const SpatialGDK::RPCPayload&);

namespace SpatialGDK
{
class SpatialEventTracer;

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
					  USpatialLatencyTracer* SpatialLatencyTracer, SpatialEventTracer* EventTracer);

	EPushRPCResult PushRPC(FEntityId EntityId, ERPCType Type, RPCPayload Payload, bool bCreatedEntity, UObject* Target = nullptr,
						   UFunction* Function = nullptr);
	void PushOverflowedRPCs();

	struct UpdateToSend
	{
		FEntityId EntityId;
		FWorkerComponentUpdate Update;
		TOptional<Trace_SpanId> SpanId;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<FWorkerComponentData> GetRPCComponentsOnEntityCreation(FEntityId EntityId);

	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCsForEntity(FEntityId EntityId, Worker_ComponentId ComponentId);

	// Will also store acked IDs locally.
	void IncrementAckedRPCID(FEntityId EntityId, ERPCType Type);

	void OnCheckoutMulticastRPCComponentOnEntity(FEntityId EntityId);
	void OnRemoveMulticastRPCComponentForEntity(FEntityId EntityId);

	void OnEndpointAuthorityGained(FEntityId EntityId, Worker_ComponentId ComponentId);
	void OnEndpointAuthorityLost(FEntityId EntityId, Worker_ComponentId ComponentId);

	uint64 GetLastAckedRPCId(FEntityId EntityId, ERPCType Type) const;

private:
	struct PendingRPCPayload
	{
		PendingRPCPayload(const RPCPayload& InPayload)
			: Payload(InPayload)
		{
		}

		RPCPayload Payload;
		TOptional<Trace_SpanId> SpanId;
	};

	// For now, we should drop overflowed RPCs when entity crosses the boundary.
	// When locking works as intended, we should re-evaluate how this will work (drop after some time?).
	void ClearOverflowedRPCs(FEntityId EntityId);

	EPushRPCResult PushRPCInternal(FEntityId EntityId, ERPCType Type, PendingRPCPayload&& Payload, bool bCreatedEntity);

	void ExtractRPCsForType(FEntityId EntityId, ERPCType Type);

	void AddOverflowedRPC(EntityRPCType EntityType, PendingRPCPayload&& Payload);

	uint64 GetAckFromView(FEntityId EntityId, ERPCType Type);
	const RPCRingBuffer& GetBufferFromView(FEntityId EntityId, ERPCType Type);

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair, const Trace_SpanId* SpanId);
	Schema_ComponentData* GetOrCreateComponentData(EntityComponentId EntityComponentIdPair);

private:
	ExtractRPCDelegate ExtractRPCCallback;
	const USpatialStaticComponentView* View;
	USpatialLatencyTracer* SpatialLatencyTracer;
	SpatialEventTracer* EventTracer;

	// This is local, not written into schema.
	TMap<FEntityId, uint64> LastSeenMulticastRPCIds;
	TMap<EntityRPCType, uint64> LastSeenRPCIds;

	// Stored here for things we have authority over.
	TMap<EntityRPCType, uint64> LastAckedRPCIds;
	TMap<EntityRPCType, uint64> LastSentRPCIds;

	TMap<EntityComponentId, Schema_ComponentData*> PendingRPCsOnEntityCreation;

	struct PendingUpdate
	{
		PendingUpdate(Schema_ComponentUpdate* InUpdate)
			: Update(InUpdate)
		{
		}

		Schema_ComponentUpdate* Update;
		TArray<Trace_SpanId> SpanIds;
	};

	TMap<EntityComponentId, PendingUpdate> PendingComponentUpdatesToSend;
	TMap<EntityRPCType, TArray<PendingRPCPayload>> OverflowedRPCs;

#if TRACE_LIB_ACTIVE
	void ProcessResultToLatencyTrace(const EPushRPCResult Result, const TraceKey Trace);
	TMap<EntityComponentId, TraceKey> PendingTraces;
#endif
};

} // namespace SpatialGDK
