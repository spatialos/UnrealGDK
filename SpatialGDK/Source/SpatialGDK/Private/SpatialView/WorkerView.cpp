// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/WorkerView.h"
#include "SpatialView/MessagesToSend.h"

namespace SpatialGDK
{

WorkerView::WorkerView()
: LocalChanges(MakeUnique<MessagesToSend>())
{
}

const ViewDelta* WorkerView::GenerateViewDelta()
{
	Delta.Clear();
	for (auto& OpList : QueuedOps)
	{
		Delta.AddOpList(MoveTemp(OpList), AddedComponents);
	}
	QueuedOps.Empty();
	return &Delta;
}

void WorkerView::EnqueueOpList(TUniquePtr<AbstractOpList> OpList)
{
	QueuedOps.Push(MoveTemp(OpList));
}

TUniquePtr<MessagesToSend> WorkerView::FlushLocalChanges()
{
	TUniquePtr<MessagesToSend> OutgoingMessages = MoveTemp(LocalChanges);
	LocalChanges = MakeUnique<MessagesToSend>();
	return OutgoingMessages;
}

void WorkerView::SendAddComponent(Worker_EntityId EntityId, ComponentData Data)
{
	AddedComponents.Add(EntityComponentId{ EntityId, Data.GetComponentId() });
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Data));
}

void WorkerView::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update)
{
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Update));
}

void WorkerView::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	AddedComponents.Remove(EntityComponentId{ EntityId, ComponentId });
	LocalChanges->ComponentMessages.Emplace(EntityId, ComponentId);
}

void WorkerView::SendCreateEntityRequest(CreateEntityRequest Request)
{
	LocalChanges->CreateEntityRequests.Push(MoveTemp(Request));
}

}  // namespace SpatialGDK
