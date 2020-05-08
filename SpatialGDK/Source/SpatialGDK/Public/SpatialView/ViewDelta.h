// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "SpatialView/AuthorityRecord.h"
#include "SpatialView/CommandMessages.h"
#include "SpatialView/EntityComponentRecord.h"
#include "SpatialView/OpList/AbstractOpList.h"
#include "Containers/Array.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class ViewDelta
{
public:
	void AddCreateEntityResponse(CreateEntityResponse Response);

	void SetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Worker_Authority Authority);
	void AddComponent(Worker_EntityId EntityId, ComponentData Data);
	void RemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void AddComponentAsUpdate(Worker_EntityId EntityId, ComponentData Data);
	void AddUpdate(Worker_EntityId EntityId, ComponentUpdate Update);

	const TArray<CreateEntityResponse>& GetCreateEntityResponses() const;
	const TArray<EntityComponentId>& GetAuthorityGained() const;
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;
	const TArray<EntityComponentData>& GetComponentsAdded() const;
	const TArray<EntityComponentId>& GetComponentsRemoved() const;
	const TArray<EntityComponentUpdate>& GetUpdates() const;
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

	// Returns an array of ops equivalent to the current state of the view delta.
	// It is expected that Clear should be called between calls to GenerateLegacyOpList.
	// todo Remove this once the view delta is not read via a legacy op list.
	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

	void Clear();

private:
	// todo wrap world command responses in their own record?
	TArray<CreateEntityResponse> CreateEntityResponses;

	AuthorityRecord AuthorityChanges;
	EntityComponentRecord EntityComponentChanges;
};

}  // namespace SpatialGDK
