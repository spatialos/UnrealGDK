// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewCoordinator.h"
#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

namespace SpatialGDK
{

ViewCoordinator::ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler)
	: ConnectionHandler(MoveTemp(ConnectionHandler)), NextRequestId(1)
{
	Delta = View.GenerateViewDelta();
}

void ViewCoordinator::Advance()
{
	ConnectionHandler->Advance();
	const uint32 OpListCount = ConnectionHandler->GetOpListCount();
	for (uint32 i = 0; i < OpListCount; ++i)
	{
		View.EnqueueOpList(ConnectionHandler->GetNextOpList());
	}
	Delta = View.GenerateViewDelta();
}

void ViewCoordinator::FlushMessagesToSend()
{
	ConnectionHandler->SendMessages(View.FlushLocalChanges());
}

bool ViewCoordinator::HasDisconnected() const
{
	return Delta->HasDisconnected();
}

uint8 ViewCoordinator::GetConnectionStatus() const
{
	return Delta->GetConnectionStatus();
}

FString ViewCoordinator::GetConnectionReason() const
{
	return Delta->GetDisconnectReason();
}

void ViewCoordinator::SendAddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	View.SendAddComponent(EntityId, MoveTemp(Data));
}

void ViewCoordinator::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	View.SendComponentUpdate(EntityId, MoveTemp(Update));
}

void ViewCoordinator::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	View.SendRemoveComponent(EntityId, ComponentId);
}

Worker_RequestId ViewCoordinator::SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis)
{
	View.SendReserveEntityIdsRequest({NextRequestId, NumberOfEntityIds, TimeoutMillis});
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendCreateEntityRequest(TArray<ComponentData> EntityComponents,
	TOptional<Worker_EntityId> EntityId, TOptional<uint32> TimeoutMillis)
{
	View.SendCreateEntityRequest({NextRequestId, MoveTemp(EntityComponents), EntityId, TimeoutMillis});
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis)
{
	View.SendDeleteEntityRequest({NextRequestId, EntityId, TimeoutMillis});
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis)
{
	View.SendEntityQueryRequest({NextRequestId, MoveTemp(Query), TimeoutMillis});
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request,
	TOptional<uint32> TimeoutMillis)
{
	View.SendEntityCommandRequest({EntityId, NextRequestId, MoveTemp(Request), TimeoutMillis});
	return NextRequestId++;
}

void ViewCoordinator::SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response)
{
	View.SendEntityCommandResponse({RequestId, MoveTemp(Response)});
}

void ViewCoordinator::SendEntityCommandFailure(Worker_RequestId RequestId, FString Message)
{
	View.SendEntityCommandFailure({RequestId, MoveTemp(Message)});
}

void ViewCoordinator::SendMetrics(SpatialMetrics Metrics)
{
	View.SendMetrics(MoveTemp(Metrics));
}

OpList ViewCoordinator::GetLegacyOpList() const
{
	return GetOpListFromViewDelta(*Delta);
}

const TArray<Worker_EntityId>& ViewCoordinator::GetEntitiesAdded() const
{
	return Delta->GetEntitiesAdded();
}

const TArray<Worker_EntityId>& ViewCoordinator::GetEntitiesRemoved() const
{
	return Delta->GetEntitiesRemoved();
}

const TArray<EntityComponentId>& ViewCoordinator::GetAuthorityGained() const
{
	return Delta->GetAuthorityGained();
}

const TArray<EntityComponentId>& ViewCoordinator::GetAuthorityLost() const
{
	return Delta->GetAuthorityLost();
}

const TArray<EntityComponentId>& ViewCoordinator::GetAuthorityLostTemporarily() const
{
	return Delta->GetAuthorityLostTemporarily();
}

const TArray<EntityComponentData>& ViewCoordinator::GetComponentsAdded() const
{
	return Delta->GetComponentsAdded();
}

const TArray<EntityComponentId>& ViewCoordinator::GetComponentsRemoved() const
{
	return Delta->GetComponentsRemoved();
}

const TArray<EntityComponentUpdate>& ViewCoordinator::GetUpdates() const
{
	return Delta->GetUpdates();
}

const TArray<EntityComponentCompleteUpdate>& ViewCoordinator::GetCompleteUpdates() const
{
	return Delta->GetCompleteUpdates();
}

const TArray<Worker_Op>& ViewCoordinator::GetWorkerMessages() const
{
	return Delta->GetWorkerMessages();
}

}  // namespace SpatialGDK
