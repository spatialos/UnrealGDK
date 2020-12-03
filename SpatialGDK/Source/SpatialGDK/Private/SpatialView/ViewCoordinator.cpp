// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewCoordinator.h"

#include "SpatialView/OpList/ViewDeltaLegacyOpList.h"

namespace SpatialGDK
{
ViewCoordinator::ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler, TSharedPtr<SpatialEventTracer> EventTracer,
								 FComponentSetData ComponentSetData)
	: View(MoveTemp(ComponentSetData))
	, ConnectionHandler(MoveTemp(ConnectionHandler))
	, NextRequestId(1)
	, ReceivedOpEventHandler(MoveTemp(EventTracer))
{
}

ViewCoordinator::~ViewCoordinator()
{
	FlushMessagesToSend();
}

void ViewCoordinator::Advance(float DeltaTimeS)
{
	// Get new op lists.
	ConnectionHandler->Advance();
	const uint32 OpListCount = ConnectionHandler->GetOpListCount();

	// Hold back open critical sections.
	for (uint32 i = 0; i < OpListCount; ++i)
	{
		OpList Ops = ConnectionHandler->GetNextOpList();
		ReserveEntityIdRetryHandler.ProcessOps(DeltaTimeS, Ops, View);
		CreateEntityRetryHandler.ProcessOps(DeltaTimeS, Ops, View);
		DeleteEntityRetryHandler.ProcessOps(DeltaTimeS, Ops, View);
		EntityQueryRetryHandler.ProcessOps(DeltaTimeS, Ops, View);
		EntityCommandRetryHandler.ProcessOps(DeltaTimeS, Ops, View);
		CriticalSectionFilter.AddOpList(MoveTemp(Ops));
	}

	// Process ops.
	TArray<OpList> OpLists = CriticalSectionFilter.GetReadyOpLists();
	for (const OpList& Ops : OpLists)
	{
		ReceivedOpEventHandler.ProcessOpLists(Ops);
	}
	View.AdvanceViewDelta(MoveTemp(OpLists));

	// Process the view delta.
	Dispatcher.InvokeCallbacks(View.GetViewDelta().GetEntityDeltas());
	for (const TUniquePtr<FSubView>& SubviewToAdvance : SubViews)
	{
		SubviewToAdvance->Advance(View.GetViewDelta());
	}
}

const ViewDelta& ViewCoordinator::GetViewDelta() const
{
	return View.GetViewDelta();
}

const EntityView& ViewCoordinator::GetView() const
{
	return View.GetView();
}

void ViewCoordinator::FlushMessagesToSend()
{
	ConnectionHandler->SendMessages(View.FlushLocalChanges());
}

FSubView& ViewCoordinator::CreateSubView(Worker_ComponentId Tag, const FFilterPredicate& Filter,
										 const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks)
{
	const int Index = SubViews.Emplace(MakeUnique<FSubView>(Tag, Filter, &View.GetView(), Dispatcher, DispatcherRefreshCallbacks));
	return *SubViews[Index];
}

void ViewCoordinator::RefreshEntityCompleteness(Worker_EntityId EntityId)
{
	for (const TUniquePtr<FSubView>& SubviewToRefresh : SubViews)
	{
		SubviewToRefresh->RefreshEntity(EntityId);
	}
}

void ViewCoordinator::SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const FSpatialGDKSpanId& SpanId)
{
	View.SendAddComponent(EntityId, MoveTemp(Data), SpanId);
}

void ViewCoordinator::SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const FSpatialGDKSpanId& SpanId)
{
	View.SendComponentUpdate(EntityId, MoveTemp(Update), SpanId);
}

void ViewCoordinator::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const FSpatialGDKSpanId& SpanId)
{
	View.SendRemoveComponent(EntityId, ComponentId, SpanId);
}

Worker_RequestId ViewCoordinator::SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis)
{
	View.SendReserveEntityIdsRequest({ NextRequestId, NumberOfEntityIds, TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
														  TOptional<uint32> TimeoutMillis, const FSpatialGDKSpanId& SpanId)
{
	View.SendCreateEntityRequest({ NextRequestId, MoveTemp(EntityComponents), EntityId, TimeoutMillis, SpanId });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis,
														  const FSpatialGDKSpanId& SpanId)
{
	View.SendDeleteEntityRequest({ NextRequestId, EntityId, TimeoutMillis, SpanId });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis)
{
	View.SendEntityQueryRequest({ NextRequestId, MoveTemp(Query), TimeoutMillis });
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request,
														   TOptional<uint32> TimeoutMillis, const FSpatialGDKSpanId& SpanId)
{
	View.SendEntityCommandRequest({ EntityId, NextRequestId, MoveTemp(Request), TimeoutMillis, SpanId });
	return NextRequestId++;
}

void ViewCoordinator::SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response, const FSpatialGDKSpanId& SpanId)
{
	View.SendEntityCommandResponse({ RequestId, MoveTemp(Response), SpanId });
}

void ViewCoordinator::SendEntityCommandFailure(Worker_RequestId RequestId, FString Message, const FSpatialGDKSpanId& SpanId)
{
	View.SendEntityCommandFailure({ RequestId, MoveTemp(Message), SpanId });
}

void ViewCoordinator::SendMetrics(SpatialMetrics Metrics)
{
	View.SendMetrics(MoveTemp(Metrics));
}

void ViewCoordinator::SendLogMessage(Worker_LogLevel Level, const FName& LoggerName, FString Message)
{
	View.SendLogMessage({ Level, LoggerName, MoveTemp(Message) });
}

Worker_RequestId ViewCoordinator::SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, FRetryData RetryData)
{
	ReserveEntityIdRetryHandler.SendRequest(NextRequestId, NumberOfEntityIds, RetryData, View);
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
														  FRetryData RetryData, const FSpatialGDKSpanId& SpanId)
{
	CreateEntityRetryHandler.SendRequest(NextRequestId, { MoveTemp(EntityComponents), EntityId, SpanId }, RetryData, View);
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendDeleteEntityRequest(Worker_EntityId EntityId, FRetryData RetryData, const FSpatialGDKSpanId& SpanId)
{
	DeleteEntityRetryHandler.SendRequest(NextRequestId, { EntityId, SpanId }, RetryData, View);
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityQueryRequest(EntityQuery Query, FRetryData RetryData)
{
	EntityQueryRetryHandler.SendRequest(NextRequestId, MoveTemp(Query), RetryData, View);
	return NextRequestId++;
}

Worker_RequestId ViewCoordinator::SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request, FRetryData RetryData,
														   const FSpatialGDKSpanId& SpanId)
{
	EntityCommandRetryHandler.SendRequest(NextRequestId, { EntityId, MoveTemp(Request), SpanId }, RetryData, View);
	return NextRequestId++;
}

CallbackId ViewCoordinator::RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	return Dispatcher.RegisterComponentAddedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId ViewCoordinator::RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	return Dispatcher.RegisterComponentRemovedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId ViewCoordinator::RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback)
{
	return Dispatcher.RegisterComponentValueCallback(ComponentId, MoveTemp(Callback));
}

CallbackId ViewCoordinator::RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	return Dispatcher.RegisterAuthorityGainedCallback(ComponentId, MoveTemp(Callback));
}

CallbackId ViewCoordinator::RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	return Dispatcher.RegisterAuthorityLostCallback(ComponentId, MoveTemp(Callback));
}

CallbackId ViewCoordinator::RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback)
{
	return Dispatcher.RegisterAuthorityLostTempCallback(ComponentId, MoveTemp(Callback));
}

void ViewCoordinator::RemoveCallback(CallbackId Id)
{
	Dispatcher.RemoveCallback(Id);
}

FDispatcherRefreshCallback ViewCoordinator::CreateComponentExistenceRefreshCallback(
	Worker_ComponentId ComponentId, const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return FSubView::CreateComponentExistenceRefreshCallback(Dispatcher, ComponentId, RefreshPredicate);
}

FDispatcherRefreshCallback ViewCoordinator::CreateComponentChangedRefreshCallback(Worker_ComponentId ComponentId,
																				  const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return FSubView::CreateComponentChangedRefreshCallback(Dispatcher, ComponentId, RefreshPredicate);
}

FDispatcherRefreshCallback ViewCoordinator::CreateAuthorityChangeRefreshCallback(Worker_ComponentId ComponentId,
																				 const FAuthorityChangeRefreshPredicate& RefreshPredicate)
{
	return FSubView::CreateAuthorityChangeRefreshCallback(Dispatcher, ComponentId, RefreshPredicate);
}

const FString& ViewCoordinator::GetWorkerId() const
{
	return ConnectionHandler->GetWorkerId();
}

Worker_EntityId ViewCoordinator::GetWorkerSystemEntityId() const
{
	return ConnectionHandler->GetWorkerSystemEntityId();
}

} // namespace SpatialGDK
