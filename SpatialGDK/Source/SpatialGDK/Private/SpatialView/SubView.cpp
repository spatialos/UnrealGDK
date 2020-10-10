// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/SubView.h"

#include "Utils/ComponentFactory.h"

namespace SpatialGDK
{
const FFilterPredicate FSubView::NoFilter = [](const FEntityId&, const EntityViewElement&) {
	return true;
};
const TArray<FDispatcherRefreshCallback> FSubView::NoDispatcherCallbacks = TArray<FDispatcherRefreshCallback>{};
const FComponentChangeRefreshPredicate FSubView::NoComponentChangeRefreshPredicate = [](const FEntityComponentChange&) {
	return true;
};
const FAuthorityChangeRefreshPredicate FSubView::NoAuthorityChangeRefreshPredicate = [](const FEntityId) {
	return true;
};

FSubView::FSubView(const FComponentId InTagComponentId, FFilterPredicate InFilter, const EntityView* InView, FDispatcher& Dispatcher,
				   const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks)
	: TagComponentId(InTagComponentId)
	, Filter(MoveTemp(InFilter))
	, View(InView)
{
	RegisterTagCallbacks(Dispatcher);
	RegisterRefreshCallbacks(DispatcherRefreshCallbacks);
}

void FSubView::Advance(const ViewDelta& Delta)
{
	// Note: Complete entities will be a longer list than the others for the majority of iterations under
	// probable normal usage. This sort could then become expensive, and a potential optimisation would be
	// to maintain the ordering of complete entities when merging in the newly complete entities and enforcing
	// that complete entities is always sorted. This would also need to be enforced in the temporarily incomplete case.
	// If this sort shows up in a profile it would be worth trying.
	Algo::Sort(CompleteEntities);
	Algo::Sort(NewlyCompleteEntities);
	Algo::Sort(NewlyIncompleteEntities);
	Algo::Sort(TemporarilyIncompleteEntities);

	Delta.Project(SubViewDelta, CompleteEntities, NewlyCompleteEntities, NewlyIncompleteEntities, TemporarilyIncompleteEntities);

	CompleteEntities.Append(NewlyCompleteEntities);
	NewlyCompleteEntities.Empty();
	NewlyIncompleteEntities.Empty();
	TemporarilyIncompleteEntities.Empty();
}

const FSubViewDelta& FSubView::GetViewDelta() const
{
	return SubViewDelta;
}

void FSubView::RefreshEntity(const FEntityId EntityId)
{
	if (TaggedEntities.Contains(EntityId))
	{
		CheckEntityAgainstFilter(EntityId);
	}
}

const EntityView& FSubView::GetView() const
{
	return *View;
}

FDispatcherRefreshCallback FSubView::CreateComponentExistenceRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																			 const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		Dispatcher.RegisterComponentAddedCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
			if (RefreshPredicate(Change))
			{
				Callback(Change.EntityId);
			}
		});
		Dispatcher.RegisterComponentRemovedCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
			if (RefreshPredicate(Change))
			{
				Callback(Change.EntityId);
			}
		});
	};
}

FDispatcherRefreshCallback FSubView::CreateComponentChangedRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																		   const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		Dispatcher.RegisterComponentValueCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
			if (RefreshPredicate(Change))
			{
				Callback(Change.EntityId);
			}
		});
	};
}

FDispatcherRefreshCallback FSubView::CreateAuthorityChangeRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																		  const FAuthorityChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		Dispatcher.RegisterAuthorityGainedCallback(ComponentId, [RefreshPredicate, Callback](const FEntityId Id) {
			if (RefreshPredicate(Id))
			{
				Callback(Id);
			}
		});
		Dispatcher.RegisterAuthorityLostCallback(ComponentId, [RefreshPredicate, Callback](const FEntityId Id) {
			if (RefreshPredicate(Id))
			{
				Callback(Id);
			}
		});
	};
}

void FSubView::RegisterTagCallbacks(FDispatcher& Dispatcher)
{
	Dispatcher.RegisterAndInvokeComponentAddedCallback(
		TagComponentId,
		[this](const FEntityComponentChange& Change) {
			OnTaggedEntityAdded(Change.EntityId);
		},
		*View);

	Dispatcher.RegisterComponentRemovedCallback(TagComponentId, [this](const FEntityComponentChange& Change) {
		OnTaggedEntityRemoved(Change.EntityId);
	});
}

void FSubView::RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks)
{
	const FRefreshCallback RefreshEntityCallback = [this](const FEntityId EntityId) {
		RefreshEntity(EntityId);
	};
	for (FDispatcherRefreshCallback Callback : DispatcherRefreshCallbacks)
	{
		Callback(RefreshEntityCallback);
	}
}

void FSubView::OnTaggedEntityAdded(const FEntityId EntityId)
{
	TaggedEntities.Add(EntityId);
	CheckEntityAgainstFilter(EntityId);
}

void FSubView::OnTaggedEntityRemoved(const FEntityId EntityId)
{
	TaggedEntities.RemoveSingleSwap(EntityId);
	EntityIncomplete(EntityId);
}

void FSubView::CheckEntityAgainstFilter(const FEntityId EntityId)
{
	if (Filter(EntityId, (*View)[EntityId]))
	{
		EntityComplete(EntityId);
		return;
	}
	EntityIncomplete(EntityId);
}

void FSubView::EntityComplete(const FEntityId EntityId)
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
	if (!NewlyCompleteEntities.Contains(EntityId) && !CompleteEntities.Contains(EntityId))
	{
		NewlyCompleteEntities.Add(EntityId);
	}
}

void FSubView::EntityIncomplete(const FEntityId EntityId)
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
