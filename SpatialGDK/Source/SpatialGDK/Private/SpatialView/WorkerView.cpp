// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/WorkerView.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/MessagesToSend.h"
#include "SpatialView/OpList/SplitOpList.h"

namespace SpatialGDK
{
WorkerView::WorkerView(SpatialEventTracer* InEventTracer)
	: LocalChanges(MakeUnique<MessagesToSend>())
	, Delta(InEventTracer)
	, EventTracer(InEventTracer)
{
}

void WorkerView::AdvanceViewDelta()
{
	Delta.Clear();
	Delta.SetFromOpList(MoveTemp(QueuedOps), View);
	QueuedOps.Empty();
}

const ViewDelta& WorkerView::GetViewDelta() const
{
	return Delta;
}

const EntityView& WorkerView::GetView() const
{
	return View;
}

void WorkerView::EnqueueOpList(OpList Ops)
{
	// Ensure that we only process closed critical sections.
	// Scan backwards looking for critical sections ops.
	for (uint32 i = Ops.Count; i > 0; --i)
	{
		Worker_Op& Op = Ops.Ops[i - 1];
		if (Op.op_type != WORKER_OP_TYPE_CRITICAL_SECTION)
		{
			continue;
		}

		// There can only be one critical section open at a time.
		// So any previous open critical section must now be closed.
		for (OpList& OpenCriticalSection : OpenCriticalSectionOps)
		{
			QueuedOps.Add(MoveTemp(OpenCriticalSection));
		}
		OpenCriticalSectionOps.Empty();

		// If critical section op is opening the section then enqueue any ops before this point and store the open critical section.
		if (Op.op.critical_section.in_critical_section)
		{
			SplitOpListPair SplitOpLists(MoveTemp(Ops), i);
			QueuedOps.Add(MoveTemp(SplitOpLists.Head));
			OpenCriticalSectionOps.Add(MoveTemp(SplitOpLists.Tail));
		}
		// If critical section op is closing the section then enqueue all ops.
		else
		{
			QueuedOps.Add(MoveTemp(Ops));
		}
		return;
	}

	// If no critical section is present then either add this to existing open section ops if there are any or enqueue if not.
	if (OpenCriticalSectionOps.Num())
	{
		OpenCriticalSectionOps.Push(MoveTemp(Ops));
	}
	else
	{
		QueuedOps.Push(MoveTemp(Ops));
	}
}

TUniquePtr<MessagesToSend> WorkerView::FlushLocalChanges()
{
	TUniquePtr<MessagesToSend> OutgoingMessages = MoveTemp(LocalChanges);
	LocalChanges = MakeUnique<MessagesToSend>();
	return OutgoingMessages;
}

void WorkerView::SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const TOptional<worker::c::Trace_SpanId>& SpanId)
{
	EntityViewElement& Element = View.FindChecked(EntityId);
	Element.Components.Emplace(Data.DeepCopy());
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Data), SpanId);
}

void WorkerView::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const TOptional<worker::c::Trace_SpanId>& SpanId)
{
	EntityViewElement& Element = View.FindChecked(EntityId);
	ComponentData* Component = Element.Components.FindByPredicate(ComponentIdEquality{ Update.GetComponentId() });
	// check(Component != nullptr);
	if (Component != nullptr)
	{
		Component->ApplyUpdate(Update);
	}
	LocalChanges->ComponentMessages.Emplace(EntityId, MoveTemp(Update), SpanId);
}

void WorkerView::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
									 const TOptional<worker::c::Trace_SpanId>& SpanId)
{
	EntityViewElement& Element = View.FindChecked(EntityId);
	ComponentData* Component = Element.Components.FindByPredicate(ComponentIdEquality{ ComponentId });
	check(Component != nullptr);
	Element.Components.RemoveAtSwap(Component - Element.Components.GetData());
	LocalChanges->ComponentMessages.Emplace(EntityId, ComponentId, SpanId);
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
