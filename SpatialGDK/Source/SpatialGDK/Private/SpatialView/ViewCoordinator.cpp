// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{

ViewCoordinator::ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler)
: ConnectionHandler(MoveTemp(ConnectionHandler))
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

const TArray<CreateEntityResponse>& ViewCoordinator::GetCreateEntityResponses() const
{
	return Delta->GetCreateEntityResponses();
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

TUniquePtr<AbstractOpList> ViewCoordinator::GenerateLegacyOpList() const
{
	return Delta->GenerateLegacyOpList();
}

}  // namespace SpatialGDK
