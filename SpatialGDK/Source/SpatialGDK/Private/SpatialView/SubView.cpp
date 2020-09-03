// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/SubView.h"

#include "Utils/ComponentFactory.h"

namespace SpatialGDK
{
SubView::SubView(const Worker_ComponentId TagComponentId, const TFunction<bool(const Worker_EntityId, const EntityViewElement&)> Filter,
				 const EntityView& View, FDispatcher& Dispatcher)
	: TagComponentId(TagComponentId)
	, Filter(Filter)
	, View(View)
	, TaggedEntities(TSet<Worker_EntityId>{})
	, CompleteEntities(TSet<Worker_EntityId>{})
	, NewlyCompleteEntities(TSet<Worker_EntityId>{})
	, NewlyIncompleteEntities(TSet<Worker_EntityId>{})
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

void SubView::TagQuery(Query& QueryToTag) const
{
	QueryConstraint TagConstraint;
	TagConstraint.ComponentConstraint = TagComponentId;

	QueryConstraint NewConstraint;
	NewConstraint.AndConstraint.Add(QueryToTag.Constraint);
	NewConstraint.AndConstraint.Add(TagConstraint);

	QueryToTag.Constraint = NewConstraint;
	QueryToTag.ResultComponentIds.Add(TagComponentId);
}

void SubView::TagEntity(TArray<FWorkerComponentData>& Components) const
{
	Components.Add(ComponentFactory::CreateEmptyComponentData(TagComponentId));
}

void SubView::AdvanceViewDelta(const ViewDelta& Delta)
{
	// TODO: Filtering given delta by complete entities and add/removes for newly complete and incomplete entities
	// respectively. Probably requires new view delta functionality to build from worker messages and entity deltas.
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

void SubView::OnTaggedEntityAdded(const Worker_EntityId EntityId)
{
	TaggedEntities.Add(EntityId);
	CheckEntityAgainstFilter(EntityId);
}

void SubView::OnTaggedEntityRemoved(const Worker_EntityId EntityId)
{
	TaggedEntities.Remove(EntityId);
	CompleteEntities.Remove(EntityId);
}

void SubView::CheckEntityAgainstFilter(const Worker_EntityId EntityId)
{
	if (Filter(EntityId, View[EntityId]))
	{
		bool* AlreadyInSet = nullptr;
		CompleteEntities.Add(EntityId, AlreadyInSet);
		if (!AlreadyInSet)
		{
			NewlyCompleteEntities.Add(EntityId);
		}
		return;
	}
	if (CompleteEntities.Contains(EntityId))
	{
		NewlyIncompleteEntities.Add(EntityId);
		CompleteEntities.Remove(EntityId);
	}
}
} // namespace SpatialGDK
