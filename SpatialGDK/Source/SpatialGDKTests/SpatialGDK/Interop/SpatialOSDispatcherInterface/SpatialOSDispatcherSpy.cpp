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

void SpatialOSDispatcherSpy::OnAuthorityChange(const Worker_AuthorityChangeOp& Op) {}

void SpatialOSDispatcherSpy::OnComponentUpdate(const Worker_ComponentUpdateOp& Op) {}

// This gets bound to a delegate in SpatialRPCService and is called for each RPC extracted when calling
// SpatialRPCService::ExtractRPCsForEntity.
bool SpatialOSDispatcherSpy::OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
{
	return false;
}

void SpatialOSDispatcherSpy::OnCommandRequest(const Worker_CommandRequestOp& Op) {}

void SpatialOSDispatcherSpy::OnCommandResponse(const Worker_CommandResponseOp& Op) {}

void SpatialOSDispatcherSpy::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op) {}

void SpatialOSDispatcherSpy::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op) {}

void SpatialOSDispatcherSpy::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel) {}

void SpatialOSDispatcherSpy::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) {}

void SpatialOSDispatcherSpy::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{
	EntityQueryDelegates.Add(RequestId, Delegate);
}

void SpatialOSDispatcherSpy::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate) {}

void SpatialOSDispatcherSpy::AddCreateEntityDelegate(Worker_RequestId RequestId, CreateEntityDelegate Delegate) {}

void SpatialOSDispatcherSpy::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) {}

EntityQueryDelegate* SpatialOSDispatcherSpy::GetEntityQueryDelegate(Worker_RequestId RequestId)
{
	return EntityQueryDelegates.Find(RequestId);
}
