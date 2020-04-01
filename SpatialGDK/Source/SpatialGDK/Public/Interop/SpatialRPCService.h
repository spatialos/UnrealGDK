// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/RPCPayload.h"
#include "SpatialView/EntityComponentId.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialRPCService, Log, All);

class USpatialStaticComponentView;
struct RPCRingBuffer;

DECLARE_DELEGATE_RetVal_ThreeParams(bool, ExtractRPCDelegate, Worker_EntityId, ERPCType, const SpatialGDK::RPCPayload&);

namespace SpatialGDK
{
// The RPC must have either succeeded with no subsequent action to take, or failed with an indicator of the action to take
#define VALIDATE_RPC_PUSH_RESULT(Result) check((PushRPCResultUtils::GetOutcome(Result) == EPushRPCResult::Success && !PushRPCResultUtils::IsAnyFlagSet(Result, EPushRPCResult::AllFailureActions)) || \
(PushRPCResultUtils::IsAnyFlagSet(Result, EPushRPCResult::AllFailureOutcomes) && PushRPCResultUtils::IsAnyFlagSet(Result, EPushRPCResult::AllFailureActions)))

struct EntityRPCType
{
	EntityRPCType(Worker_EntityId EntityId, ERPCType Type)
		: EntityId(EntityId)
		, Type(Type)
	{}

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
	None = 0x00,
	// First group of flags tracks the Outcome (success/failure reason) of an attempt to push an RPC
	Success = 0x01,
	Overflowed = 0x02,
	HasAckAuthority = 0x04,
	NoRingBufferAuthority = 0x08,
	AlreadyQueued = 0x10,
	// Next group of flags tracks the Action to take as a result of a failure outcome
	Drop = 0x20,
	Queue = 0x40,
	// Useful combinations of flags
	AllFailureOutcomes = Overflowed | HasAckAuthority | NoRingBufferAuthority | AlreadyQueued,
	AllOutcomes = AllFailureOutcomes | Success,
	AllFailureActions = Drop | Queue
};

ENUM_CLASS_FLAGS(EPushRPCResult);

class SPATIALGDK_API SpatialRPCService
{
public:
	SpatialRPCService(ExtractRPCDelegate ExtractRPCCallback, const USpatialStaticComponentView* View);

	EPushRPCResult PushRPC(Worker_EntityId EntityId, ERPCType Type, RPCPayload Payload);
	void PushQueuedRPCs();

	struct UpdateToSend
	{
		Worker_EntityId EntityId;
		FWorkerComponentUpdate Update;
	};
	TArray<UpdateToSend> GetRPCsAndAcksToSend();
	TArray<Worker_ComponentData> GetRPCComponentsOnEntityCreation(Worker_EntityId EntityId);

	// Will also store acked IDs locally.
	// Calls ExtractRPCCallback for each RPC it extracts from a given component. If the callback returns false,
	// stops retrieving RPCs.
	void ExtractRPCsForEntity(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	void OnCheckoutMulticastRPCComponentOnEntity(Worker_EntityId EntityId);
	void OnRemoveMulticastRPCComponentForEntity(Worker_EntityId EntityId);

	void OnEndpointAuthorityGained(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void OnEndpointAuthorityLost(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	// This is specifically for whether or not we should queue RPCs on the sender
	void SetPushRPCFailureOutcomesToQueue(const EPushRPCResult FailureOutcomes);

private:
	// For now, we should drop overflowed RPCs when entity crosses the boundary.
	// When locking works as intended, we should re-evaluate how this will work (drop after some time?).
	void ClearQueuedRPCs(Worker_EntityId EntityId);

	EPushRPCResult PushRPCInternal(Worker_EntityId EntityId, ERPCType Type, RPCPayload&& Payload);

	void ExtractRPCsForType(Worker_EntityId EntityId, ERPCType Type);

	void AddQueuedRPC(EntityRPCType EntityType, RPCPayload&& Payload);

	uint64 GetAckFromView(Worker_EntityId EntityId, ERPCType Type);
	const RPCRingBuffer& GetBufferFromView(Worker_EntityId EntityId, ERPCType Type);

	Schema_ComponentUpdate* GetOrCreateComponentUpdate(EntityComponentId EntityComponentIdPair);
	Schema_ComponentData* GetOrCreateComponentData(EntityComponentId EntityComponentIdPair);

private:
	ExtractRPCDelegate ExtractRPCCallback;
	const USpatialStaticComponentView* View;

	// This is local, not written into schema.
	TMap<Worker_EntityId_Key, uint64> LastSeenMulticastRPCIds;

	// Stored here for things we have authority over.
	TMap<EntityRPCType, uint64> LastAckedRPCIds;
	TMap<EntityRPCType, uint64> LastSentRPCIds;

	TMap<EntityComponentId, Schema_ComponentData*> PendingRPCsOnEntityCreation;

	TMap<EntityComponentId, Schema_ComponentUpdate*> PendingComponentUpdatesToSend;
	TMap<EntityRPCType, TArray<RPCPayload>> QueuedRPCs;

	EPushRPCResult PushRPCFailureOutcomesToQueue;
};

namespace PushRPCResultUtils
{
	bool IsAnyFlagSet(const EPushRPCResult Result, const EPushRPCResult Flags);
	EPushRPCResult GetFailureAction(const EPushRPCResult Result);
	EPushRPCResult GetOutcome(const EPushRPCResult Result);
	EPushRPCResult MakeFailureResultCode(const EPushRPCResult Result, const ERPCType Type, const EPushRPCResult FailureOutcomesToQueue);
	bool ShouldQueueRPC(const EPushRPCResult Result, const ERPCType Type, const EPushRPCResult FailureOutcomesToQueue);
}

} // namespace SpatialGDK
