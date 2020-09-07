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
	SubView(const Worker_ComponentId TagComponentId, const FFilterPredicate Filter, const EntityView& View, FDispatcher& Dispatcher,
			const TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks);

	void TagQuery(Query& QueryToTag) const;
	void TagEntity(TArray<FWorkerComponentData>& Components) const;

	void Advance(const ViewDelta& Delta);
	const ViewDelta& GetViewDelta() const;
	void RefreshEntity(const Worker_EntityId EntityId);

	static FDispatcherRefreshCallback CreateComponentExistenceDispatcherRefreshCallback(FDispatcher& Dispatcher, Worker_ComponentId ComponentId, FComponentChangeRefreshPredicate RefreshPredicate);
	static FDispatcherRefreshCallback CreateComponentChangedDispatcherRefreshCallback(FDispatcher& Dispatcher, Worker_ComponentId ComponentId, FComponentChangeRefreshPredicate RefreshPredicate);
	static FDispatcherRefreshCallback CreateAuthorityChangeDispatcherRefreshCallback(FDispatcher& Dispatcher, Worker_ComponentId ComponentId, FAuthorityChangeRefreshPredicate RefreshPredicate);

private:
	void RegisterTagCallbacks(FDispatcher& Dispatcher);
	void RegisterRefreshCallbacks(const TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks);
	void OnTaggedEntityAdded(const Worker_EntityId EntityId);
	void OnTaggedEntityRemoved(const Worker_EntityId EntityId);
	void CheckEntityAgainstFilter(const Worker_EntityId EntityId);
	void EntityComplete(const Worker_EntityId EntityId);
	void EntityIncomplete(const Worker_EntityId EntityId);

	const Worker_ComponentId TagComponentId;
	const FFilterPredicate Filter;
	const EntityView& View;

	ViewDelta SubDelta;

	TArray<Worker_EntityId> TaggedEntities;
	TArray<Worker_EntityId> CompleteEntities;
	TArray<Worker_EntityId> NewlyCompleteEntities;
	TArray<Worker_EntityId> NewlyIncompleteEntities;
	TArray<Worker_EntityId> TemporarilyIncompleteEntities;
};

} // namespace SpatialGDK
