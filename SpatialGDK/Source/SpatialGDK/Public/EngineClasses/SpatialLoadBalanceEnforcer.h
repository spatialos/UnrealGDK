// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;
class USpatialStaticComponentView;

using AclWriteAuthorityRequestType = TPair<Worker_EntityId, FString>;
 
class SpatialLoadBalanceEnforcer
{
public:
	SpatialLoadBalanceEnforcer();

	void Init(const FString &InWorkerId, USpatialStaticComponentView* InStaticComponentView, TSharedPtr<SpatialVirtualWorkerTranslator> InVirtualWorkerTranslator);

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp);
	void QueueAclAssignmentRequest(const Worker_EntityId EntityId);

	void OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op);

	TArray<AclWriteAuthorityRequestType> ProcessQueuedAclAssignmentRequests();

private:

	FString WorkerId;
	TWeakObjectPtr<USpatialStaticComponentView> StaticComponentView;
	TWeakPtr<SpatialVirtualWorkerTranslator> VirtualWorkerTranslator;

	struct WriteAuthAssignmentRequest
	{
		WriteAuthAssignmentRequest(Worker_EntityId InputEntityId)
			: EntityId(InputEntityId)
			, ProcessAttempts(0)
		{}
		Worker_EntityId EntityId;
		int32 ProcessAttempts;
	};

	TArray<WriteAuthAssignmentRequest> AclWriteAuthAssignmentRequests;
};
