// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Dispatcher.h"
#include "EntityView.h"
#include "Schema/Interest.h"
#include "Templates/Function.h"

using FFilterPredicate = TFunction<bool(const Worker_EntityId, const SpatialGDK::EntityViewElement&)>;
using FRefreshCallback = TFunction<void(const Worker_EntityId)>;
using FDispatcherRefreshCallback = TFunction<void(const FRefreshCallback)>;
using FComponentChangeRefreshPredicate = TFunction<bool(SpatialGDK::FEntityComponentChange)>;
using FAuthorityChangeRefreshPredicate = TFunction<bool(Worker_EntityId)>;

namespace SpatialGDK
{
class FSubView
{
public:
	static const FFilterPredicate NoFilter;
	static const TArray<FDispatcherRefreshCallback> NoDispatcherCallbacks;
	static const FComponentChangeRefreshPredicate NoComponentChangeRefreshPredicate;
	static const FAuthorityChangeRefreshPredicate NoAuthorityChangeRefreshPredicate;

	// The subview constructor takes the filter and the dispatcher refresh callbacks for the subview, rather than
	// adding them to the subview later. This is to maintain the invariant that a subview always has the correct
	// full set of complete entities. During construction, it calculates the initial set of complete entities,
	// and registers the passed dispatcher callbacks in order to ensure all possible changes which could change
	// the state of completeness for any entity are picked up by the subview to maintain this invariant.
	FSubView(const Worker_ComponentId InTagComponentId, FFilterPredicate InFilter, const EntityView* InView, FDispatcher& Dispatcher,
			 const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);

	~FSubView() = default;

	// Non-copyable, non-movable
	FSubView(const FSubView&) = delete;
	FSubView(FSubView&&) = default;
	FSubView& operator=(const FSubView&) = delete;
	FSubView& operator=(FSubView&&) = default;

	void Advance(const ViewDelta& Delta);
	const FSubViewDelta& GetViewDelta() const;
	void RefreshEntity(const Worker_EntityId EntityId);

	const EntityView& GetView() const;

	// Helper functions for creating dispatcher refresh callbacks for use when constructing a subview.
	// Takes an optional predicate argument to further filter what causes a refresh. Example: Only trigger
	// a refresh if the received component change has a change for a certain field.
	static FDispatcherRefreshCallback CreateComponentExistenceRefreshCallback(FDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																			  const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateComponentChangedRefreshCallback(FDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																			const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateAuthorityChangeRefreshCallback(FDispatcher& Dispatcher, const Worker_ComponentId ComponentId,
																		   const FAuthorityChangeRefreshPredicate& RefreshPredicate);

private:
	void RegisterTagCallbacks(FDispatcher& Dispatcher);
	void RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);
	void OnTaggedEntityAdded(const Worker_EntityId EntityId);
	void OnTaggedEntityRemoved(const Worker_EntityId EntityId);
	void CheckEntityAgainstFilter(const Worker_EntityId EntityId);
	void EntityComplete(const Worker_EntityId EntityId);
	void EntityIncomplete(const Worker_EntityId EntityId);

	Worker_ComponentId TagComponentId;
	FFilterPredicate Filter;
	const EntityView* View;

	FSubViewDelta SubViewDelta;

	TArray<Worker_EntityId> TaggedEntities;
	TArray<Worker_EntityId> CompleteEntities;
	TArray<Worker_EntityId> NewlyCompleteEntities;
	TArray<Worker_EntityId> NewlyIncompleteEntities;
	TArray<Worker_EntityId> TemporarilyIncompleteEntities;
};

} // namespace SpatialGDK
