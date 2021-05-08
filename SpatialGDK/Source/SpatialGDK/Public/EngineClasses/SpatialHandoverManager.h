// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/AuthorityIntent.h"
#include "Schema/StandardLibrary.h"
#include "SpatialCommonTypes.h"

#include "SpatialView/EntityComponentTypes.h"

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
class FSubView;

struct LBComponents2
{
	AuthorityIntentV2 Intent;
	AuthorityIntentACK IntentACK;
};

class SpatialHandoverManager
{
public:
	SpatialHandoverManager(const FSubView& InActorView, const FSubView& InPartitionView);

	void Advance();

	const TSet<Worker_EntityId>& GetActorsToHandover() { return ActorsToHandover; }

	void Flush(SpatialOSWorkerInterface* Connection, const TSet<Worker_EntityId>& ActorsReleased);

private:
	void PopulateDataStore(const Worker_EntityId EntityId);
	void ApplyComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ApplyComponentRefresh(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void HandleChange(Worker_EntityId EntityId, const LBComponents2& Components);

	const FSubView& ActorView;
	const FSubView& PartitionView;

	TArray<Worker_EntityId> PendingEntityAuthorityChanges;
	TMap<Worker_EntityId_Key, LBComponents2> DataStore;
	TUniqueFunction<void(EntityComponentUpdate)> UpdateSender;

	TSet<Worker_EntityId> PartitionsToACK;
	TSet<Worker_EntityId> OwnedPartitions;
	TSet<Worker_EntityId> ActorsToHandover;
	TSet<Worker_EntityId> ActorsToACK;
};

} // namespace SpatialGDK
