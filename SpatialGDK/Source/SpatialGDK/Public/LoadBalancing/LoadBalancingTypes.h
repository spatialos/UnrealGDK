// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
struct FLBWorkerInternalState;
struct FLBWorkerDesc : TSharedFromThis<FLBWorkerDesc>
{
	FLBWorkerDesc(FName InWorkerType);
	~FLBWorkerDesc();
	const FName WorkerType;
	TUniquePtr<FLBWorkerInternalState> State;
};
using FLBWorkerHandle = TSharedPtr<FLBWorkerDesc>;

struct FPartitionInternalState;
struct FPartitionDesc : TSharedFromThis<FPartitionDesc>
{
	~FPartitionDesc();
	TUniquePtr<FPartitionInternalState> State;
};
using FPartitionHandle = TSharedPtr<FPartitionDesc>;

struct FMigrationContext
{
	FMigrationContext(const TSet<Worker_EntityId>& InMigratingEntities, const TSet<Worker_EntityId>& InModifiedEntities)
		: MigratingEntities(InMigratingEntities)
		, ModifiedEntities(InModifiedEntities)
	{
	}

	const TSet<Worker_EntityId>& MigratingEntities;
	const TSet<Worker_EntityId>& ModifiedEntities;
	TMap<Worker_EntityId, FPartitionHandle> EntitiesToMigrate;
};

} // namespace SpatialGDK
