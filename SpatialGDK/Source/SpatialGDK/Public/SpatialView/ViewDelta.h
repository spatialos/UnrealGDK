// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/AuthorityRecord.h"
#include "SpatialView/EntityComponentRecord.h"
#include "SpatialView/EntityPresenceRecord.h"
#include "SpatialView/OpList/OpList.h"
#include <WorkerSDK/improbable/c_worker.h>

#include "Containers/Array.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
class ViewDelta
{
public:
	void AddOpList(OpList Ops, TSet<EntityComponentId>& ComponentsPresent);

	bool HasDisconnected() const;
	uint8 GetConnectionStatus() const;
	FString GetDisconnectReason() const;

	const TArray<Worker_EntityId>& GetEntitiesAdded() const;
	const TArray<Worker_EntityId>& GetEntitiesRemoved() const;
	const TArray<EntityComponentId>& GetAuthorityGained() const;
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;
	const TArray<EntityComponentData>& GetComponentsAdded() const;
	const TArray<EntityComponentId>& GetComponentsRemoved() const;
	const TArray<EntityComponentUpdate>& GetUpdates() const;
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

	const TArray<Worker_Op>& GetWorkerMessages() const;

	void Clear();

private:
	void ProcessOp(const Worker_Op& Op, TSet<EntityComponentId>& ComponentsPresent);

	void HandleAuthorityChange(const Worker_AuthorityChangeOp& Op);
	void HandleAddComponent(const Worker_AddComponentOp& Op, TSet<EntityComponentId>& ComponentsPresent);
	void HandleComponentUpdate(const Worker_ComponentUpdateOp& Op);
	void HandleRemoveComponent(const Worker_RemoveComponentOp& Op, TSet<EntityComponentId>& ComponentsPresent);

	TArray<Worker_Op> WorkerMessages;

	AuthorityRecord AuthorityChanges;
	EntityPresenceRecord EntityPresenceChanges;
	EntityComponentRecord EntityComponentChanges;

	TArray<OpList> OpLists;

	uint8 ConnectionStatus = 0;
	FString DisconnectReason;
};

} // namespace SpatialGDK
