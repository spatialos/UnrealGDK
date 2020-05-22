// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "SpatialView/AuthorityRecord.h"
#include "SpatialView/EntityComponentRecord.h"
#include "SpatialView/EntityPresenceRecord.h"
#include "SpatialView/OpList/AbstractOpList.h"
#include "Containers/Array.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>


namespace SpatialGDK
{

class ViewDelta
{
public:
	void AddOpList(TUniquePtr<AbstractOpList> OpList, TSet<EntityComponentId>& ComponentsPresent);

	const TArray<EntityComponentId>& GetAuthorityGained() const;
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;
	const TArray<EntityComponentData>& GetComponentsAdded() const;
	const TArray<EntityComponentId>& GetComponentsRemoved() const;
	const TArray<EntityComponentUpdate>& GetUpdates() const;
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

	const TArray<Worker_Op>& GetWorkerMessages() const;

	// Returns an array of ops equivalent to the current state of the view delta.
	// It is expected that Clear should be called between calls to GenerateLegacyOpList.
	// todo Remove this once the view delta is not read via a legacy op list.
	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

	void Clear();

private:
	void ProcessOp(const Worker_Op& Op, TSet<EntityComponentId>& ComponentsPresent);

	void HandleAuthorityChange(const Worker_AuthorityChangeOp& AuthorityChange);
	void HandleAddComponent(const Worker_AddComponentOp& Component, TSet<EntityComponentId>& ComponentsPresent);
	void HandleComponentUpdate(const Worker_ComponentUpdateOp& Update);
	void HandleRemoveComponent(const Worker_RemoveComponentOp& Component, TSet<EntityComponentId>& ComponentsPresent);

	TArray<Worker_Op> WorkerMessages;

	AuthorityRecord AuthorityChanges;
	EntityPresenceRecord EntityPresenceChanges;
	EntityComponentRecord EntityComponentChanges;

	TArray<TUniquePtr<AbstractOpList>> OpLists;
};

}  // namespace SpatialGDK
