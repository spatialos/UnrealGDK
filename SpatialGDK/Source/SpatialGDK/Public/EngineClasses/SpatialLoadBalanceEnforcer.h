// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialLoadBalanceEnforcer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class USpatialNetDriver;
class SpatialVirtualWorkerTranslator;

UCLASS()
class USpatialLoadBalanceEnforcer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void Init(USpatialNetDriver* InNetDriver, SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator);
	void Tick();

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp);
	void QueueAclAssignmentRequest(const Worker_EntityId EntityId);

	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);

private:

	USpatialNetDriver* NetDriver;
	SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	struct WriteAuthAssignmentRequest
	{
		WriteAuthAssignmentRequest(Worker_EntityId InputEntityId)
			: EntityId(InputEntityId)
			, ProcessAttempts(0)
		{}
		Worker_EntityId EntityId;
		int16_t ProcessAttempts;
	};

	TArray<WriteAuthAssignmentRequest> AclWriteAuthAssignmentRequests;

	void ProcessQueuedAclAssignmentRequests();
	void SetAclWriteAuthority(const Worker_EntityId EntityId, const FString& WorkerId);
};
