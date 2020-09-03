// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "Dispatcher.h"
#include "EntityView.h"
#include "Schema/Interest.h"
#include "Templates/Function.h"

namespace SpatialGDK
{
	class SubView
	{
	public:
		SubView(const Worker_ComponentId TagComponentId,
			const TFunction<bool(const Worker_EntityId, const EntityViewElement&)> Filter,
			const EntityView& View,
			FDispatcher& Dispatcher);

		void TagQuery(Query& QueryToTag) const;
		void TagEntity(TArray<FWorkerComponentData>& Components) const;

		void AdvanceViewDelta(const ViewDelta& Delta);
		const ViewDelta& GetViewDelta() const;
		void RefreshEntity(const Worker_EntityId EntityId);

	private:
		void OnTaggedEntityAdded(const Worker_EntityId EntityId);
		void OnTaggedEntityRemoved(const Worker_EntityId EntityId);
		void CheckEntityAgainstFilter(const Worker_EntityId EntityId);

		const Worker_ComponentId TagComponentId;
		const TFunction<bool(const Worker_EntityId, const EntityViewElement&)> Filter;
		const EntityView& View;

		ViewDelta SubDelta;

		TSet<Worker_EntityId> TaggedEntities;
		TSet<Worker_EntityId> CompleteEntities;
		TSet<Worker_EntityId> NewlyCompleteEntities;
		TSet<Worker_EntityId> NewlyIncompleteEntities;
	};

} // namespace SpatialGDK
