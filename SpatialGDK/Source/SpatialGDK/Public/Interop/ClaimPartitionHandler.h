// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

class SpatialOSWorkerInterface;

using SystemEntityCommandDelegate = TFunction<void(const Worker_CommandResponseOp&)>;

namespace SpatialGDK
{
class ClaimPartitionHandler
{
public:
	ClaimPartitionHandler(SpatialOSWorkerInterface& InWorkerInterface);

	void ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim);
	void ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim, SystemEntityCommandDelegate Delegate);

	void ProcessOps(const TArray<Worker_Op>& Ops);

private:
	struct ClaimPartitionRequest
	{
		Worker_PartitionId PartitionId;
		SystemEntityCommandDelegate Delegate;
	};

	TMap<Worker_RequestId_Key, ClaimPartitionRequest> ClaimPartitionRequestIds;

	SpatialOSWorkerInterface& WorkerInterface;
};
} // namespace SpatialGDK
