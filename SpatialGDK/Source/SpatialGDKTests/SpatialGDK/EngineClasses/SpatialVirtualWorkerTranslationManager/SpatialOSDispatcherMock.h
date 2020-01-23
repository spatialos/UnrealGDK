// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialOSDispatcherInterface.h"

class SpatialOSDispatcherMock : public SpatialOSDispatcherInterface
{
public:
	SpatialOSDispatcherMock();

	// Dispatcher Calls
	virtual void OnCriticalSection(bool InCriticalSection) override;
	virtual void OnAddEntity(const Worker_AddEntityOp& Op) override;
	virtual void OnAddComponent(const Worker_AddComponentOp& Op) override;
	virtual void OnRemoveEntity(const Worker_RemoveEntityOp& Op) override;
	virtual void OnRemoveComponent(const Worker_RemoveComponentOp& Op) override;
	virtual void FlushRemoveComponentOps() override;
	virtual void RemoveComponentOpsForEntity(Worker_EntityId EntityId) override;
	virtual void OnAuthorityChange(const Worker_AuthorityChangeOp& Op) override;

	virtual void OnComponentUpdate(const Worker_ComponentUpdateOp& Op) override;

	// This gets bound to a delegate in SpatialRPCService and is called for each RPC extracted when calling SpatialRPCService::ExtractRPCsForEntity.
	virtual bool OnExtractIncomingRPC(Worker_EntityId EntityId, ERPCType RPCType, const SpatialGDK::RPCPayload& Payload) override;

	virtual void OnCommandRequest(const Worker_CommandRequestOp& Op) override;
	virtual void OnCommandResponse(const Worker_CommandResponseOp& Op) override;

	virtual void OnReserveEntityIdsResponse(const Worker_ReserveEntityIdsResponseOp& Op) override;
	virtual void OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op) override;

	virtual void AddPendingActorRequest(Worker_RequestId RequestId, USpatialActorChannel* Channel) override;
	virtual void AddPendingReliableRPC(Worker_RequestId RequestId, TSharedRef<struct FReliableRPCForRetry> ReliableRPC) override;

	virtual void AddEntityQueryDelegate(Worker_RequestId RequestId, EntityQueryDelegate Delegate) override;
	virtual void AddReserveEntityIdsDelegate(Worker_RequestId RequestId, ReserveEntityIDsDelegate Delegate) override;
	virtual void AddCreateEntityDelegate(Worker_RequestId RequestId, const CreateEntityDelegate& Delegate) override;

	virtual void OnEntityQueryResponse(const Worker_EntityQueryResponseOp& Op) override;

private:
};
