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
	for (auto& Ops : QueuedOps)
	{
		Delta.AddOpList(MoveTemp(Ops), AddedComponents);
	}
	QueuedOps.Empty();
	return &Delta;
}

void WorkerView::EnqueueOpList(OpList Ops)
{
	QueuedOps.Push(MoveTemp(Ops));
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

void WorkerView::SendReserveEntityIdsRequest(ReserveEntityIdsRequest Request)
{
	LocalChanges->ReserveEntityIdsRequests.Push(MoveTemp(Request));
}

void WorkerView::SendCreateEntityRequest(CreateEntityRequest Request)
{
	LocalChanges->CreateEntityRequests.Push(MoveTemp(Request));
}

void WorkerView::SendDeleteEntityRequest(DeleteEntityRequest Request)
{
	LocalChanges->DeleteEntityRequests.Push(MoveTemp(Request));
}

void WorkerView::SendEntityQueryRequest(EntityQueryRequest Request)
{
	LocalChanges->EntityQueryRequests.Push(MoveTemp(Request));
}

void WorkerView::SendEntityCommandRequest(EntityCommandRequest Request)
{
	LocalChanges->EntityCommandRequests.Push(MoveTemp(Request));
}

void WorkerView::SendEntityCommandResponse(EntityCommandResponse Response)
{
	LocalChanges->EntityCommandResponses.Push(MoveTemp(Response));
}

void WorkerView::SendEntityCommandFailure(EntityCommandFailure Failure)
{
	LocalChanges->EntityCommandFailures.Push(MoveTemp(Failure));
}

void WorkerView::SendMetrics(SpatialMetrics Metrics)
{
	LocalChanges->Metrics.Add(MoveTemp(Metrics));
}

}  // namespace SpatialGDK
