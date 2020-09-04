// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Dispatcher.h"
#include "EntityView.h"
#include "Schema/Interest.h"
#include "Templates/Function.h"

typedef TFunction<bool(const Worker_EntityId, const SpatialGDK::EntityViewElement&)> FFilterPredicate;
typedef TFunction<void(const Worker_EntityId)> FRefreshCallback;
typedef TFunction<void(const FRefreshCallback)> FDispatcherRefreshCallback;

namespace SpatialGDK
{
class SubView
{
public:
	SubView(const Worker_ComponentId TagComponentId, const FFilterPredicate Filter, const EntityView& View, FDispatcher& Dispatcher,
			const TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks);

	void TagQuery(Query& QueryToTag) const;
	void TagEntity(TArray<FWorkerComponentData>& Components) const;

	void AdvanceViewDelta(const ViewDelta& Delta);
	const ViewDelta& GetViewDelta() const;
	void RefreshEntity(const Worker_EntityId EntityId);

private:
	void RegisterTagCallbacks(const EntityView& View, FDispatcher& Dispatcher);
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
	// TODO: TempIncompleteEntities
};

} // namespace SpatialGDK
