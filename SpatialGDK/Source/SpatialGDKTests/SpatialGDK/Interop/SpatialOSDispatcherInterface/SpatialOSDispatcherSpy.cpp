// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSDispatcherSpy.h"

SpatialOSDispatcherSpy::SpatialOSDispatcherSpy() {}

// Dispatcher Calls
void SpatialOSDispatcherSpy::OnCriticalSection(bool InCriticalSection) {}

void SpatialOSDispatcherSpy::OnAddEntity(const Worker_AddEntityOp& Op) {}

void SpatialOSDispatcherSpy::OnAddComponent(const Worker_AddComponentOp& Op) {}

void SpatialOSDispatcherSpy::OnRemoveEntity(const Worker_RemoveEntityOp& Op) {}

void SpatialOSDispatcherSpy::OnRemoveComponent(const Worker_RemoveComponentOp& Op) {}

void SpatialOSDispatcherSpy::FlushRemoveComponentOps() {}

void SpatialOSDispatcherSpy::DropQueuedRemoveComponentOpsForEntity(Worker_EntityId EntityId) {}

void SpatialOSDispatcherSpy::OnAuthorityChange(const Worker_ComponentSetAuthorityChangeOp& Op) {}

void SpatialOSDispatcherSpy::OnComponentUpdate(const Worker_ComponentUpdateOp& Op) {}

// This gets bound to a delegate in SpatialRPCService and is called for each RPC extracted when calling
// SpatialRPCService::ExtractRPCsForEntity.
bool SpatialOSDispatcherSpy::OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
{
	return false;
}

void SpatialOSDispatcherSpy::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) {}

void SpatialOSDispatcherSpy::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, Delegate);
}

void SpatialOSDispatcherSpy::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) {}

EntityQueryDelegate* SpatialOSDispatcherSpy::GetEntityQueryDelegate(Worker_RequestId RequestId)
{
	return EntityQueryDelegates.Find(RequestId);
}
