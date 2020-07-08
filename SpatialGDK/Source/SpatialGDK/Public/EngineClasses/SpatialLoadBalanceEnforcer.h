// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

class SPATIALGDK_API SpatialLoadBalanceEnforcer
{
public:
	struct AuthorityStateChange
	{
		Worker_EntityId EntityId = 0;
		WorkerRequirementSet ReadAcl;
		TArray<Worker_ComponentId> ComponentIds;
		VirtualWorkerId TargetVirtualWorker;
	};

	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const USpatialStaticComponentView* InStaticComponentView, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator);

	static bool HandlesComponent(Worker_ComponentId ComponentId);

	void OnLoadBalancingComponentAdded(const Worker_AddComponentOp& Op);
	void OnLoadBalancingComponentUpdated(const Worker_ComponentUpdateOp& Op);
	void OnLoadBalancingComponentRemoved(const Worker_RemoveComponentOp& Op);
	void OnEntityRemoved(const Worker_RemoveEntityOp& Op);
	void OnAclAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp);

	void MaybeQueueAuthorityChange(const Worker_EntityId EntityId);
	bool EntityNeedsToBeEnforced(const Worker_EntityId EntityId) const;

	// Visible for testing
	bool AclAssignmentRequestIsQueued(const Worker_EntityId EntityId) const;
	bool GetAuthorityChangeState(Worker_EntityId EntityId, AuthorityStateChange& OutAuthorityChange) const;

	TArray<AuthorityStateChange> ProcessAuthorityChangeRequests();

private:
	bool CanEnforce(Worker_EntityId EntityId) const;

	const PhysicalWorkerName WorkerId;
	TWeakObjectPtr<const USpatialStaticComponentView> StaticComponentView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	TArray<Worker_EntityId> PendingEntityAuthorityChanges;
};
