// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/RPCPayload.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

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
	virtual void DropQueuedRemoveComponentOpsForEntity(Worker_EntityId EntityId)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::DropQueuedRemoveComponentOpsForEntity, return;);
	virtual void OnAuthorityChange(const Worker_ComponentSetAuthorityChangeOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnAuthorityChange, return;);
	virtual void OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnComponentUpdate, return;);
	virtual bool OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::OnExtractIncomingRPC, return false;);

	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC)
		PURE_VIRTUAL(SpatialOSDispatcherInterface::AddPendingReliableRPC, return;);
};
