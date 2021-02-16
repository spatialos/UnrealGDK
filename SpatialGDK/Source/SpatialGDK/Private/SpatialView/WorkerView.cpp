// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/WorkerView.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/MessagesToSend.h"
#include "SpatialView/OpList/SplitOpList.h"

namespace SpatialGDK
{
WorkerView::WorkerView(FComponentSetData ComponentSetData)
	: ComponentSetData(MoveTemp(ComponentSetData))
	, LocalChanges(MakeUnique<MessagesToSend>())
{
}

void WorkerView::AdvanceViewDelta(TArray<OpList> OpLists)
{
	Delta.SetFromOpList(MoveTemp(OpLists), View, ComponentSetData);
}

const ViewDelta& WorkerView::GetViewDelta() const
{
	return Delta;
}

const EntityView& WorkerView::GetView() const
{
	return View;
}

TUniquePtr<MessagesToSend> WorkerView::FlushLocalChanges()
{
	TUniquePtr<MessagesToSend> OutgoingMessages = MoveTemp(LocalChanges);
	LocalChanges = MakeUnique<MessagesToSend>();
	return OutgoingMessages;
}

void WorkerView::SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const FSpatialGDKSpanId& SpanId)
{
	EntityViewElement* Element = View.Find(EntityId);
	if (ensure(Element != nullptr))
	{
		Element->Components.Emplace(Data.DeepCopy());
		LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Data), SpanId);
	}
}

void WorkerView::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const FSpatialGDKSpanId& SpanId)
{
	EntityViewElement* Element = View.Find(EntityId);
	if (ensure(Element != nullptr))
	{
		ComponentData* Component = Element->Components.FindByPredicate(ComponentIdEquality{ Update.GetComponentId() });
		if (Component != nullptr)
		{
			Component->ApplyUpdate(Update);
		}
		LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Update), SpanId);
	}
}

void WorkerView::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId)
{
	EntityViewElement* Element = View.Find(EntityId);
	if (ensure(Element != nullptr))
	{
		ComponentData* Component = Element->Components.FindByPredicate(ComponentIdEquality{ ComponentId });
		check(Component != nullptr);
		Element->Components.RemoveAtSwap(Component - Element->Components.GetData());
		LocalChanges->ComponentMessages.Emplace(EntityId, ComponentId, SpanId);
	}
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

void WorkerView::SendLogMessage(LogMessage Log)
{
	LocalChanges->Logs.Add(MoveTemp(Log));
}

} // namespace SpatialGDK
