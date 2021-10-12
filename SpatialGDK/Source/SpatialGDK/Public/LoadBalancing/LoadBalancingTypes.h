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

class FActorSetSystem;
class FPartitionManager;
class InterestFactory;

// Some abstractions from SpatialOS and Unreal that are expected to be available to all load balancing systems.
struct FLoadBalancingSharedData
{
	FLoadBalancingSharedData(FPartitionManager& InPartitionManager, FActorSetSystem& InActorSets, InterestFactory& InInterestF)
		: PartitionManager(InPartitionManager)
		, ActorSets(InActorSets)
		, InterestF(InInterestF)
	{
	}

	FPartitionManager& PartitionManager;
	FActorSetSystem& ActorSets;
	InterestFactory& InterestF;
};

struct FMigrationContext
{
	FMigrationContext(const TSet<Worker_EntityId_Key>& InMigratingEntities, const TSet<Worker_EntityId_Key>& InModifiedEntities,
					  const TSet<Worker_EntityId_Key>& InDeletedEntities)
		: MigratingEntities(InMigratingEntities)
		, ModifiedEntities(InModifiedEntities)
		, DeletedEntities(InDeletedEntities)
	{
	}
	const TSet<Worker_EntityId_Key>& MigratingEntities;
	const TSet<Worker_EntityId_Key>& ModifiedEntities;
	const TSet<Worker_EntityId_Key>& DeletedEntities;
	TMap<Worker_EntityId_Key, FPartitionHandle> EntitiesToMigrate;
};

} // namespace SpatialGDK
