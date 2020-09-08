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
class SubView
{
public:
	// The subview constructor takes the filter and the dispatcher refresh callbacks for the subview, rather than
	// adding them to the subview later. This is to maintain the invariant that a subview always has the correct
	// full set of complete entities. During construction, it calculates the initial set of complete entities,
	// and registers the passed dispatcher callbacks in order to ensure all possible changes which could change
	// the state of completeness for any entity are picked up by the subview to maintain this invariant.
	SubView(const Worker_ComponentId& TagComponentId, const FFilterPredicate& Filter, const EntityView& View, FDispatcher& Dispatcher,
			const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);

	// Non-copyable
	SubView(const SubView&) = delete;
	SubView& operator=(const SubView&) = delete;

	void TagQuery(Query& QueryToTag) const;
	void TagEntity(TArray<FWorkerComponentData>& Components) const;

	void Advance(const ViewDelta& Delta);
	const SubViewDelta& GetViewDelta() const;
	void RefreshEntity(const Worker_EntityId& EntityId);

	// Helper functions for creating dispatcher refresh callbacks for use when constructing a subview.
	// Takes an optional predicate argument to further filter what causes a refresh. Example: Only trigger
	// a refresh if the received component change has a change for a certain field.
	static FDispatcherRefreshCallback CreateComponentExistenceDispatcherRefreshCallback(
		FDispatcher& Dispatcher, const Worker_ComponentId& ComponentId, const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateComponentChangedDispatcherRefreshCallback(
		FDispatcher& Dispatcher, const Worker_ComponentId& ComponentId, const FComponentChangeRefreshPredicate& RefreshPredicate);
	static FDispatcherRefreshCallback CreateAuthorityChangeDispatcherRefreshCallback(
		FDispatcher& Dispatcher, const Worker_ComponentId& ComponentId, const FAuthorityChangeRefreshPredicate& RefreshPredicate);

private:
	void RegisterTagCallbacks(FDispatcher& Dispatcher);
	void RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);
	void OnTaggedEntityAdded(const Worker_EntityId& EntityId);
	void OnTaggedEntityRemoved(const Worker_EntityId& EntityId);
	void CheckEntityAgainstFilter(const Worker_EntityId& EntityId);
	void EntityComplete(const Worker_EntityId& EntityId);
	void EntityIncomplete(const Worker_EntityId& EntityId);

	const Worker_ComponentId TagComponentId;
	const FFilterPredicate Filter;
	const EntityView& View;

	SubViewDelta SubDelta;

	TArray<Worker_EntityId> TaggedEntities;
	TArray<Worker_EntityId> CompleteEntities;
	TArray<Worker_EntityId> NewlyCompleteEntities;
	TArray<Worker_EntityId> NewlyIncompleteEntities;
	TArray<Worker_EntityId> TemporarilyIncompleteEntities;
};

} // namespace SpatialGDK
