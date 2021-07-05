// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "Templates/Function.h"

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
class ClaimPartitionHandler
{
public:
	using FPartitionClaimCallback = TFunction<void(void)>;

	ClaimPartitionHandler(SpatialOSWorkerInterface& InWorkerInterface);

	void ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim,
						FPartitionClaimCallback InCallback = FPartitionClaimCallback());

	void ProcessOps(const TArray<Worker_Op>& Ops);

private:
	struct FPartitionClaimRequest
	{
		Worker_PartitionId PartitionId;
		FPartitionClaimCallback Callback;
	};
	TMap<Worker_RequestId_Key, FPartitionClaimRequest> ClaimPartitionRequestIds;

	SpatialOSWorkerInterface& WorkerInterface;
};
} // namespace SpatialGDK
