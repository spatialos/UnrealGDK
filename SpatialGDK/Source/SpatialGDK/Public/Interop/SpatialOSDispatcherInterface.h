// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "Schema/RPCPayload.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_DELEGATE_OneParam(EntityQueryDelegate, const Worker_EntityQueryResponseOp&);
DECLARE_DELEGATE_OneParam(ReserveEntityIDsDelegate, const Worker_ReserveEntityIdsResponseOp&);
DECLARE_DELEGATE_OneParam(CreateEntityDelegate, const Worker_CreateEntityResponseOp&);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntityAddedDelegate, const FEntityId);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntityRemovedDelegate, const FEntityId);

class SpatialOSDispatcherInterface
{
public:
	// Dispatcher Calls
	virtual void OnCriticalSection(bool InCriticalSection) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnCriticalSection, return;);
	virtual void OnAddEntity(const Worker_AddEntityOp& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnAddEntity, return;);
	virtual void OnAddComponent(const Worker_AddComponentOp& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnAddComponent, return;);
	virtual void OnRemoveEntity(const Worker_RemoveEntityOp& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnRemoveEntity, return;);
	virtual void OnRemoveComponent(const Worker_RemoveComponentOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnRemoveComponent, return;);
	virtual void FlushRemoveComponentOps() PURE_VIRTUAL(SpatialOSDispatcherInterface::FlushRemoveComponentOps, return;);
	virtual void DropQueuedRemoveComponentOpsForEntity(FEntityId EntityId)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::DropQueuedRemoveComponentOpsForEntity, return;);
	virtual void OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnAuthorityChange, return;);
	virtual void OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnComponentUpdate, return;);
	virtual void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnEntityQueryResponse, return;);
	virtual bool OnExtractIncomingRPC(FEntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnExtractIncomingRPC, return false;);
	virtual void OnCommandRequest(const Worker_Op& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnCommandRequest, return;);
	virtual void OnCommandResponse(const Worker_Op& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnCommandResponse, return;);
	virtual void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnReserveEntityIdsResponse, return;);
	virtual void OnCreateEntityResponse(const Worker_Op& Op) PURE_VIRTUAL(SpatialOSDispatcherInterface::OnCreateEntityResponse, return;);

	virtual void AddPendingActorRequest(FRequestId RequestId, USpatialActorChannel* Channel)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddPendingActorRequest, return;);
	virtual void AddPendingReliableRPC(FRequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddPendingReliableRPC, return;);
	virtual void AddEntityQueryDelegate(FRequestId RequestId, EntityQueryDelegate Delegate)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddEntityQueryDelegate, return;);
	virtual void AddReserveEntityIdsDelegate(FRequestId RequestId, ReserveEntityIDsDelegate Delegate)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddReserveEntityIdsDelegate, return;);
	virtual void AddCreateEntityDelegate(FRequestId RequestId, CreateEntityDelegate Delegate)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddCreateEntityDelegate, return;);
};
