// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Dispatcher.h"
#include "EntityView.h"
#include "Templates/Function.h"

using FFilterPredicate = TFunction<bool(const FEntityId, const SpatialGDK::EntityViewElement&)>;
using FRefreshCallback = TFunction<void(const FEntityId)>;
using FDispatcherRefreshCallback = TFunction<void(const FRefreshCallback)>;
using FComponentChangeRefreshPredicate = TFunction<bool(SpatialGDK::FEntityComponentChange)>;
using FAuthorityChangeRefreshPredicate = TFunction<bool(FEntityId)>;

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
	FSubView(const FComponentId InTagComponentId, FFilterPredicate InFilter, const EntityView* InView, FDispatcher& Dispatcher,
			 const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);

	~FSubView() = default;

	// Non-copyable, non-movable
	FSubView(const FSubView&) = delete;
	FSubView(FSubView&&) = default;
	FSubView& operator=(const FSubView&) = delete;
	FSubView& operator=(FSubView&&) = default;

	void Advance(const ViewDelta& Delta);
	const FSubViewDelta& GetViewDelta() const;
	void RefreshEntity(const FEntityId EntityId);

	const EntityView& GetView() const;

	// Helper functions for creating dispatcher refresh callbacks for use when constructing a subview.
	// Takes an optional predicate argument to further filter what causes a refresh. Example: Only trigger
	// a refresh if the received component change has a change for a certain field.
	static FDispatcherRefreshCallback CreateComponentExistenceRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																			  const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateComponentChangedRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																			const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateAuthorityChangeRefreshCallback(FDispatcher& Dispatcher, const FComponentId ComponentId,
																		   const FAuthorityChangeRefreshPredicate& RefreshPredicate);

private:
	void RegisterTagCallbacks(FDispatcher& Dispatcher);
	void RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);
	void OnTaggedEntityAdded(const FEntityId EntityId);
	void OnTaggedEntityRemoved(const FEntityId EntityId);
	void CheckEntityAgainstFilter(const FEntityId EntityId);
	void EntityComplete(const FEntityId EntityId);
	void EntityIncomplete(const FEntityId EntityId);

	FComponentId TagComponentId;
	FFilterPredicate Filter;
	const EntityView* View;

	FSubViewDelta SubViewDelta;

	TArray<FEntityId> TaggedEntities;
	TArray<FEntityId> CompleteEntities;
	TArray<FEntityId> NewlyCompleteEntities;
	TArray<FEntityId> NewlyIncompleteEntities;
	TArray<FEntityId> TemporarilyIncompleteEntities;
};

} // namespace SpatialGDK
