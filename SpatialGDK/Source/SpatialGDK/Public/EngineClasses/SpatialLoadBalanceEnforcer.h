// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialLoadBalanceEnforcer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;
class USpatialSender;
class USpatialStaticComponentView;

UCLASS()
class USpatialLoadBalanceEnforcer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void Init(const FString &InWorkerId, USpatialStaticComponentView* InStaticComponentView, USpatialSender* InSpatialSender, SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator);
	void Tick();

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp);
	void QueueAclAssignmentRequest(const Worker_EntityId EntityId);

	void OnAuthorityIntentComponentUpdated(const Worker_ComponentUpdateOp& Op);

private:

	FString WorkerId;
	USpatialStaticComponentView* StaticComponentView;
	USpatialSender* Sender;
	SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

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

	void ProcessQueuedAclAssignmentRequests();
};
