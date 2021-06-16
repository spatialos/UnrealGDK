// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/SubView.h"

#include "Algo/Copy.h"
#include "SpatialView/EntityComponentTypes.h"
#include "Utils/ComponentFactory.h"

namespace SpatialGDK
{
const FFilterPredicate FSubView::NoFilter = [](const FSpatialEntityId&, const EntityViewElement&) {
	return true;
};
const TArray<FDispatcherRefreshCallback> FSubView::NoDispatcherCallbacks = TArray<FDispatcherRefreshCallback>{};
const FComponentChangeRefreshPredicate FSubView::NoComponentChangeRefreshPredicate = [](const FEntityComponentChange&) {
	return true;
};
const FAuthorityChangeRefreshPredicate FSubView::NoAuthorityChangeRefreshPredicate = [](const FSpatialEntityId) {
	return true;
};

FSubView::FSubView(const Worker_ComponentId InTagComponentId, FFilterPredicate InFilter, const EntityView* InView, IDispatcher& Dispatcher,
				   const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks)
	: TagComponentId(InTagComponentId)
	, Filter(MoveTemp(InFilter))
	, View(InView)
	, ScopedDispatcherCallbacks()
{
	RegisterTagCallbacks(Dispatcher);
	RegisterRefreshCallbacks(Dispatcher, DispatcherRefreshCallbacks);
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

TSet<Worker_EntityId_Key> FSubView::GetCompleteEntities() const
{
	TSet<Worker_EntityId_Key> CompleteEntitiesSet;

	Algo::Copy(CompleteEntities, CompleteEntitiesSet);
	Algo::Copy(NewlyCompleteEntities, CompleteEntitiesSet);

	TSet<Worker_EntityId_Key> IncompleteEntitiesSet;

	Algo::Copy(NewlyIncompleteEntities, IncompleteEntitiesSet);

	return CompleteEntitiesSet.Difference(IncompleteEntitiesSet);
}

void FSubView::Refresh()
{
	for (const Worker_EntityId_Key TaggedEntityId : TaggedEntities)
	{
		CheckEntityAgainstFilter(TaggedEntityId);
	}
}

void FSubView::RefreshEntity(const FSpatialEntityId EntityId)
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

bool FSubView::HasEntity(const FSpatialEntityId EntityId) const
{
	const EntityViewElement* Entity = View->Find(EntityId);
	return Entity != nullptr;
}

bool FSubView::IsEntityComplete(const FSpatialEntityId EntityId) const
{
	return GetCompleteEntities().Contains(EntityId);
}

bool FSubView::HasComponent(const FSpatialEntityId EntityId, const Worker_ComponentId ComponentId) const
{
	const EntityViewElement* Entity = View->Find(EntityId);
	if (Entity == nullptr)
	{
		return false;
	}
	return Entity->Components.ContainsByPredicate(ComponentIdEquality{ ComponentId });
}

bool FSubView::HasAuthority(const FSpatialEntityId EntityId, const Worker_ComponentId ComponentId) const
{
	const EntityViewElement* Entity = View->Find(EntityId);
	if (Entity == nullptr)
	{
		return false;
	}
	return Entity->Authority.Contains(ComponentId);
}

FDispatcherRefreshCallback FSubView::CreateComponentExistenceRefreshCallback(IDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																			 const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		const CallbackId AddedCallbackId =
			Dispatcher.RegisterComponentAddedCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
				if (RefreshPredicate(Change))
				{
					Callback(Change.EntityId);
				}
			});

		const CallbackId RemovedCallbackId =
			Dispatcher.RegisterComponentRemovedCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
				if (RefreshPredicate(Change))
				{
					Callback(Change.EntityId);
				}
			});
		return TArray<CallbackId>({ AddedCallbackId, RemovedCallbackId });
	};
}

FDispatcherRefreshCallback FSubView::CreateComponentChangedRefreshCallback(IDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																		   const FComponentChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		const CallbackId ValueCallbackId =
			Dispatcher.RegisterComponentValueCallback(ComponentId, [RefreshPredicate, Callback](const FEntityComponentChange& Change) {
				if (RefreshPredicate(Change))
				{
					Callback(Change.EntityId);
				}
			});
		return TArray<CallbackId>({ ValueCallbackId });
	};
}

FDispatcherRefreshCallback FSubView::CreateAuthorityChangeRefreshCallback(IDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																		  const FAuthorityChangeRefreshPredicate& RefreshPredicate)
{
	return [ComponentId, &Dispatcher, RefreshPredicate](const FRefreshCallback& Callback) {
		const CallbackId GainedCallbackId =
			Dispatcher.RegisterAuthorityGainedCallback(ComponentId, [RefreshPredicate, Callback](const FSpatialEntityId Id) {
				if (RefreshPredicate(Id))
				{
					Callback(Id);
				}
			});
		const CallbackId LostCallbackId =
			Dispatcher.RegisterAuthorityLostCallback(ComponentId, [RefreshPredicate, Callback](const FSpatialEntityId Id) {
				if (RefreshPredicate(Id))
				{
					Callback(Id);
				}
			});
		return TArray<CallbackId>({ GainedCallbackId, LostCallbackId });
	};
}

void FSubView::RegisterTagCallbacks(IDispatcher& Dispatcher)
{
	CallbackId AddedCallbackId = Dispatcher.RegisterAndInvokeComponentAddedCallback(
		TagComponentId,
		[this](const FEntityComponentChange& Change) {
			OnTaggedEntityAdded(Change.EntityId);
		},
		*View);
	ScopedDispatcherCallbacks.Emplace(Dispatcher, AddedCallbackId);

	CallbackId RemovedCallbackId =
		Dispatcher.RegisterComponentRemovedCallback(TagComponentId, [this](const FEntityComponentChange& Change) {
			OnTaggedEntityRemoved(Change.EntityId);
		});
	ScopedDispatcherCallbacks.Emplace(Dispatcher, RemovedCallbackId);
}

void FSubView::RegisterRefreshCallbacks(IDispatcher& Dispatcher, const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks)
{
	const FRefreshCallback RefreshEntityCallback = [this](const FSpatialEntityId EntityId) {
		RefreshEntity(EntityId);
	};
	for (FDispatcherRefreshCallback Callback : DispatcherRefreshCallbacks)
	{
		const TArray<CallbackId> RegisteredCallbackIds = Callback(RefreshEntityCallback);
		for (const CallbackId& RegisteredCallbackId : RegisteredCallbackIds)
		{
			ScopedDispatcherCallbacks.Emplace(Dispatcher, RegisteredCallbackId);
		}
	}
}

void FSubView::OnTaggedEntityAdded(const FSpatialEntityId EntityId)
{
	TaggedEntities.Add(EntityId);
	CheckEntityAgainstFilter(EntityId);
}

void FSubView::OnTaggedEntityRemoved(const FSpatialEntityId EntityId)
{
	TaggedEntities.RemoveSingleSwap(EntityId);
	EntityIncomplete(EntityId);
}

void FSubView::CheckEntityAgainstFilter(const FSpatialEntityId EntityId)
{
	if (View->Contains(EntityId) && Filter(EntityId, (*View)[EntityId]))
	{
		EntityComplete(EntityId);
		return;
	}
	EntityIncomplete(EntityId);
}

void FSubView::EntityComplete(const FSpatialEntityId EntityId)
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

void FSubView::EntityIncomplete(const FSpatialEntityId EntityId)
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
