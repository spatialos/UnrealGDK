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

	bool HasDisconnected() const;
	uint8 GetConnectionStatus() const;
	FString GetConnectionReason() const;

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	Worker_RequestId SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis);
	Worker_RequestId SendCreateEntityRequest(TArray<ComponentData> EntityComponents,
		TOptional<Worker_EntityId> EntityId, TOptional<uint32> TimeoutMillis);
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis);
	Worker_RequestId SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis);
	Worker_RequestId SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request,
		TOptional<uint32> TimeoutMillis);
	void SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response);
	void SendEntityCommandFailure(Worker_RequestId RequestId, FString Message);
	void SendMetrics(SpatialMetrics Metrics);

	OpList GetLegacyOpList() const;

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

private:
	const ViewDelta* Delta;
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
	Worker_RequestId NextRequestId;
};

} // namespace SpatialGDK
