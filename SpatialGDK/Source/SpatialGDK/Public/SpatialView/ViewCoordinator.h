// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/WorkerView.h"
#include "SpatialView/ConnectionHandlers/AbstractConnectionHandler.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{

class ViewCoordinator
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler);

	void Advance();
	void FlushMessagesToSend();

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);

	const TArray<CreateEntityResponse>& GetCreateEntityResponses() const;
	const TArray<EntityComponentId>& GetAuthorityGained() const;
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;
	const TArray<EntityComponentData>& GetComponentsAdded() const;
	const TArray<EntityComponentId>& GetComponentsRemoved() const;
	const TArray<EntityComponentUpdate>& GetUpdates() const;
	const TArray<EntityComponentCompleteUpdate>& GetCompleteUpdates() const;

	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

private:
	const ViewDelta* Delta;
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
};

} // namespace SpatialGDK
