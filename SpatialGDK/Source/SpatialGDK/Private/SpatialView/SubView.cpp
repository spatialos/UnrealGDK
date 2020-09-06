// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/SubView.h"

#include "Utils/ComponentFactory.h"

namespace SpatialGDK
{
SubView::SubView(const Worker_ComponentId TagComponentId, const FFilterPredicate Filter, const EntityView& View, FDispatcher& Dispatcher,
				 const TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks)
	: TagComponentId(TagComponentId)
	, Filter(Filter)
	, View(View)
	, TaggedEntities(TArray<Worker_EntityId>{})
	, CompleteEntities(TArray<Worker_EntityId>{})
	, NewlyCompleteEntities(TArray<Worker_EntityId>{})
	, NewlyIncompleteEntities(TArray<Worker_EntityId>{})
	, TemporarilyIncompleteEntities(TArray<Worker_EntityId>{})
{
	RegisterTagCallbacks(Dispatcher);
	RegisterRefreshCallbacks(DispatcherRefreshCallbacks);
}

void SubView::TagQuery(Query& QueryToTag) const
{
	QueryToTag.ResultComponentIds.Add(TagComponentId);

	QueryConstraint TagConstraint;
	TagConstraint.ComponentConstraint = TagComponentId;

	if (QueryToTag.Constraint.AndConstraint.Num() != 0)
	{
		QueryToTag.Constraint.AndConstraint.Add(TagConstraint);
		return;
	}

	QueryConstraint NewConstraint;
	NewConstraint.AndConstraint.Add(QueryToTag.Constraint);
	NewConstraint.AndConstraint.Add(TagConstraint);

	QueryToTag.Constraint = NewConstraint;
}

void SubView::TagEntity(TArray<FWorkerComponentData>& Components) const
{
	Components.Add(ComponentFactory::CreateEmptyComponentData(TagComponentId));
}

void SubView::AdvanceViewDelta(const ViewDelta& Delta)
{
	Algo::Sort(CompleteEntities);
	Algo::Sort(NewlyCompleteEntities);
	Algo::Sort(NewlyIncompleteEntities);
	Algo::Sort(TemporarilyIncompleteEntities);

	SubDelta.Project(Delta, CompleteEntities, NewlyCompleteEntities, NewlyIncompleteEntities, TemporarilyIncompleteEntities);

	CompleteEntities.Append(NewlyCompleteEntities);
	NewlyCompleteEntities.Empty();
	NewlyIncompleteEntities.Empty();
	TemporarilyIncompleteEntities.Empty();
}

const ViewDelta& SubView::GetViewDelta() const
{
	return SubDelta;
}

void SubView::RefreshEntity(const Worker_EntityId EntityId)
{
	if (TaggedEntities.Contains(EntityId))
	{
		CheckEntityAgainstFilter(EntityId);
	}
}

void SubView::RegisterTagCallbacks(FDispatcher& Dispatcher)
{
	Dispatcher.RegisterAndInvokeComponentAddedCallback(
		TagComponentId,
		[this](const FEntityComponentChange Change) {
			OnTaggedEntityAdded(Change.EntityId);
		},
		View);
	Dispatcher.RegisterComponentRemovedCallback(TagComponentId, [this](const FEntityComponentChange Change) {
		OnTaggedEntityRemoved(Change.EntityId);
	});
}

void SubView::RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks)
{
	const FRefreshCallback RefreshEntityCallback = [this](const Worker_EntityId EntityId) {
		RefreshEntity(EntityId);
	};
	for (FDispatcherRefreshCallback Callback : DispatcherRefreshCallbacks)
	{
		Callback(RefreshEntityCallback);
	}
}

void SubView::OnTaggedEntityAdded(const Worker_EntityId EntityId)
{
	TaggedEntities.Add(EntityId);
	CheckEntityAgainstFilter(EntityId);
}

void SubView::OnTaggedEntityRemoved(const Worker_EntityId EntityId)
{
	TaggedEntities.RemoveSingleSwap(EntityId);
	EntityIncomplete(EntityId);
}

void SubView::CheckEntityAgainstFilter(const Worker_EntityId EntityId)
{
	if (Filter(EntityId, View[EntityId]))
	{
		EntityComplete(EntityId);
		return;
	}
	EntityIncomplete(EntityId);
}

void SubView::EntityComplete(const Worker_EntityId EntityId)
{
	// We were just about to remove this entity, but it has become complete again before the delta was read.
	// Mark it as temporarily incomplete, but otherwise treat it as if it hadn't gone incomplete.
	if (NewlyIncompleteEntities.RemoveSingleSwap(EntityId))
	{
		CompleteEntities.Add(EntityId);
		TemporarilyIncompleteEntities.Add(EntityId);
		return;
	}
	// This is new to us. Mark it as newly complete.
	if (!CompleteEntities.Contains(EntityId))
	{
		NewlyCompleteEntities.Add(EntityId);
	}
}

void SubView::EntityIncomplete(const Worker_EntityId EntityId)
{
	// If we were about to add this, don't. It's as if we never saw it.
	if (NewlyCompleteEntities.RemoveSingleSwap(EntityId))
	{
		return;
	}
	// Otherwise, if it is currently complete, we need to remove it, and mark it as about to remove.
	if (CompleteEntities.RemoveSingleSwap(EntityId))
	{
		NewlyIncompleteEntities.Add(EntityId);
	}
}
} // namespace SpatialGDK
