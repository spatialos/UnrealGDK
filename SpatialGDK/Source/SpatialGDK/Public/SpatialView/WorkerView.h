// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/MessagesToSend.h"
#include "SpatialView/ViewDelta.h"
#include "Containers/Set.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{

class WorkerView
{
public:
	WorkerView();

	// Process queued op lists to create a new view delta.
	// The view delta will exist until the next call to advance.
	const ViewDelta* GenerateViewDelta();

	// Add an OpList to generate the next ViewDelta.
	void EnqueueOpList(TUniquePtr<AbstractOpList> OpList);

	// Ensure all local changes have been applied and return the resulting MessagesToSend.
	TUniquePtr<MessagesToSend> FlushLocalChanges();

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	void SendCreateEntityRequest(CreateEntityRequest Request);

private:
	TArray<TUniquePtr<AbstractOpList>> QueuedOps;

	ViewDelta Delta;
	TUniquePtr<MessagesToSend> LocalChanges;
	TSet<EntityComponentId> AddedComponents;
};

}  // namespace SpatialGDK
