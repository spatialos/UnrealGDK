// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/ClaimPartitionHandler.h"
#include "Interop/CreateEntityHandler.h"
#include "Interop/EntityQueryHandler.h"
#include "Interop/ReserveEntityIdsHandler.h"

namespace SpatialGDK
{
struct FCommandsHandler
{
	void ProcessOps(const TArray<Worker_Op>& Ops)
	{
		ClaimPartitionHandler.ProcessOps(Ops);
		CreateEntityHandler.ProcessOps(Ops);
		EntityQueryHandler.ProcessOps(Ops);
		ReserveEntityIdsHandler.ProcessOps(Ops);
	}

	void ClaimPartition(ISpatialOSWorker& WorkerInterface, Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim)
	{
		ClaimPartitionHandler.ClaimPartition(WorkerInterface, SystemEntityId, PartitionToClaim);
	}

	void ClaimPartition(ISpatialOSWorker& WorkerInterface, Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim,
						FSystemEntityCommandDelegate Delegate)
	{
		ClaimPartitionHandler.ClaimPartition(WorkerInterface, SystemEntityId, PartitionToClaim, MoveTemp(Delegate));
	}

	void AddRequest(Worker_RequestId RequestId, FCreateEntityDelegate Handler)
	{
		CreateEntityHandler.AddRequest(RequestId, MoveTemp(Handler));
	}

	void AddRequest(Worker_RequestId RequestId, FEntityQueryDelegate Handler)
	{
		EntityQueryHandler.AddRequest(RequestId, MoveTemp(Handler));
	}

	void AddRequest(Worker_RequestId RequestId, FReserveEntityIDsDelegate Handler)
	{
		ReserveEntityIdsHandler.AddRequest(RequestId, MoveTemp(Handler));
	}

	FClaimPartitionHandler ClaimPartitionHandler;
	FCreateEntityHandler CreateEntityHandler;
	FEntityQueryHandler EntityQueryHandler;
	FReserveEntityIdsHandler ReserveEntityIdsHandler;
};
} // namespace SpatialGDK
