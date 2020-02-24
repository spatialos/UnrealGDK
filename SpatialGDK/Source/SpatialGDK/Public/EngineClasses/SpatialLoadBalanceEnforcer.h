// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

class SpatialLoadBalanceEnforcer
{
public:
	struct AclWriteAuthorityRequest
	{
		Worker_EntityId EntityId = 0;
		PhysicalWorkerName OwningWorkerId;
		WorkerRequirementSet ReadAcl;
		WorkerRequirementSet ClientRequirementSet;
		TArray<Worker_ComponentId> ComponentIds;
	};

	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const USpatialStaticComponentView* InStaticComponentView, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator);


	void OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op);
	void OnLoadBalancingComponentAdded(const Worker_AddComponentOp& Op);
	void OnLoadBalancingComponentRemoved(const Worker_RemoveComponentOp& Op);
	void OnEntityRemoved(const Worker_RemoveEntityOp& Op);
	void OnAclAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp);

	void MaybeQueueAclAssignmentRequest(const Worker_EntityId EntityId);

	TArray<AclWriteAuthorityRequest> ProcessQueuedAclAssignmentRequests();

private:

	void QueueAclAssignmentRequest(const Worker_EntityId EntityId);
	bool AclAssignmentRequestIsQueued(const Worker_EntityId EntityId) const;
	bool CanEnforce(Worker_EntityId EntityId) const;

	const PhysicalWorkerName WorkerId;
	TWeakObjectPtr<const USpatialStaticComponentView> StaticComponentView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	TArray<Worker_EntityId> AclWriteAuthAssignmentRequests;
};
