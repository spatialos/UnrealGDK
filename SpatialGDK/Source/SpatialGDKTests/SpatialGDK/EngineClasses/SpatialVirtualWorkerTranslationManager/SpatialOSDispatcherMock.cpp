// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialOSDispatcherMock.h"

SpatialOSDispatcherMock::SpatialOSDispatcherMock()
{}

// Dispatcher Calls
void SpatialOSDispatcherMock::OnCriticalSection(bool InCriticalSection)
{}

void SpatialOSDispatcherMock::OnAddEntity(const Worker_AddEntityOp& Op)
{}

void SpatialOSDispatcherMock::OnAddComponent(const Worker_AddComponentOp& Op)
{}

void SpatialOSDispatcherMock::OnRemoveEntity(const Worker_RemoveEntityOp& Op)
{}

void SpatialOSDispatcherMock::OnRemoveComponent(const Worker_RemoveComponentOp& Op)
{}

void SpatialOSDispatcherMock::FlushRemoveComponentOps()
{}

void SpatialOSDispatcherMock::RemoveComponentOpsForEntity(Worker_EntityId EntityId)
{}

void SpatialOSDispatcherMock::OnAuthorityChange(const Worker_AuthorityChangeOp& Op)
{}

void SpatialOSDispatcherMock::OnComponentUpdate(const Worker_ComponentUpdateOp& Op)
{}

// This gets bound to a delegate in SpatialRPCService and is called for each RPC extracted when calling SpatialRPCService::ExtractRPCsForEntity.
bool SpatialOSDispatcherMock::OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload)
{
	return false;
}

void SpatialOSDispatcherMock::OnCommandRequest(const Worker_CommandRequestOp& Op)
{}

void SpatialOSDispatcherMock::OnCommandResponse(const Worker_CommandResponseOp& Op)
{}

void SpatialOSDispatcherMock::OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op)
{}

void SpatialOSDispatcherMock::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op)
{}

void SpatialOSDispatcherMock::AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel)
{}

void SpatialOSDispatcherMock::AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC)
{}

void SpatialOSDispatcherMock::AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate)
{}

void SpatialOSDispatcherMock::AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate)
{}

void SpatialOSDispatcherMock::AddCreateEntityDelegate(Worker_RequestId RequestId, const CreateEntityDelegate& Delegate)
{}

void SpatialOSDispatcherMock::OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op)
{}
