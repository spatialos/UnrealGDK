// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
class ClaimPartitionHandler
{
public:
	ClaimPartitionHandler(SpatialOSWorkerInterface& InWorkerInterface);

	void ClaimPartition(FSpatialEntityId SystemEntityId, Worker_PartitionId PartitionToClaim);

	void ProcessOps(const TArray<Worker_Op>& Ops);

private:
	TMap<Worker_RequestId_Key, Worker_PartitionId> ClaimPartitionRequestIds;

	SpatialOSWorkerInterface& WorkerInterface;
};
} // namespace SpatialGDK
