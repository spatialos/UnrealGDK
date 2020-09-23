// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewCoordinator.h"
#include "SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"
#include "Utils/ComponentFactory.h"

#define SUBVIEW_TEST(TestName) GDK_TEST(Core, SubView, TestName)

namespace SpatialGDK
{
SUBVIEW_TEST(GIVEN_Set_Of_Components_WHEN_Entity_Tagged_THEN_Components_Contains_Tag)
{
	const Worker_ComponentId TagComponentId = 1;
	const Worker_ComponentId ValueComponentId = 2;

	FDispatcher Dispatcher;
	EntityView View;
	const FSubView SubView(TagComponentId, FSubView::NoFilter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);
	TArray<FWorkerComponentData> Components{ ComponentFactory::CreateEmptyComponentData(ValueComponentId) };

	SubView.TagEntity(Components);

	TestTrue("Components contains tag component", Components.ContainsByPredicate([TagComponentId](const FWorkerComponentData& Data) {
		return Data.component_id == TagComponentId;
	}));

	return true;
}

SUBVIEW_TEST(GIVEN_Query_WHEN_Query_Tagged_THEN_Query_Is_Tagged)
{
	const Worker_EntityId EntityId = 1;
	const Worker_ComponentId TagComponentId = 1;
	const Worker_ComponentId ValueComponentId = 2;

	FDispatcher Dispatcher;
	EntityView View;
	FSubView SubView(TagComponentId, FSubView::NoFilter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);
	Query QueryToTag;
	QueryConstraint Constraint;
	Constraint.EntityIdConstraint = EntityId;
	QueryToTag.Constraint = Constraint;
	QueryToTag.ResultComponentIds = SchemaResultType{ ValueComponentId };

	SubView.TagQuery(QueryToTag);

	if (!TestNotEqual("Constraint is now an AND", QueryToTag.Constraint.AndConstraint.Num(), 0))
	{
		return true;
	}
	TArray<QueryConstraint>& AndConstraint = QueryToTag.Constraint.AndConstraint;
	Worker_ComponentId TestConstraintId = AndConstraint[0].ComponentConstraint.IsSet() ? AndConstraint[0].ComponentConstraint.GetValue()
																					   : AndConstraint[1].ComponentConstraint.GetValue();
	TestEqual("Constraint contains tag component constraint", TestConstraintId, TagComponentId);

	TestTrue("Result type contains tag", QueryToTag.ResultComponentIds.Contains(TagComponentId));

	return true;
}

SUBVIEW_TEST(GIVEN_SubView_Without_Filter_WHEN_Tagged_Entity_Added_THEN_Delta_Contains_Entity)
{
	const Worker_EntityId TaggedEntityId = 2;
	const Worker_ComponentId TagComponentId = 1;

	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	FSubView SubView(TagComponentId, FSubView::NoFilter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);

	AddEntityToView(View, TaggedEntityId);
	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });

	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	// The tagged entity should pass through to the sub view delta.
	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);

	return true;
}

SUBVIEW_TEST(
	GIVEN_SubView_With_Filter_WHEN_Tagged_Entities_Added_THEN_Delta_Only_Contains_Filtered_Entities_ALSO_Dispatcher_Callback_Refreshes_Correctly)
{
	const Worker_EntityId TaggedEntityId = 2;
	const Worker_EntityId OtherTaggedEntityId = 3;
	const Worker_ComponentId TagComponentId = 1;
	const Worker_ComponentId ValueComponentId = 2;
	const double CorrectValue = 1;
	const double IncorrectValue = 2;

	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	FFilterPredicate Filter = [ValueComponentId, CorrectValue](const Worker_EntityId&, const EntityViewElement& Element) {
		const ComponentData* It = Element.Components.FindByPredicate(ComponentIdEquality{ ValueComponentId });
		if (GetValueFromTestComponentData(It->GetUnderlying()) == CorrectValue)
		{
			return true;
		}
		return false;
	};
	auto RefreshCallbacks = TArray<FDispatcherRefreshCallback>{ FSubView::CreateComponentChangedRefreshCallback(
		Dispatcher, ValueComponentId, FSubView::NoComponentChangeRefreshPredicate) };

	FSubView SubView(TagComponentId, Filter, &View, Dispatcher, RefreshCallbacks);

	AddEntityToView(View, TaggedEntityId);
	AddComponentToView(View, TaggedEntityId, CreateTestComponentData(ValueComponentId, CorrectValue));
	AddEntityToView(View, OtherTaggedEntityId);
	AddComponentToView(View, OtherTaggedEntityId, CreateTestComponentData(ValueComponentId, IncorrectValue));

	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	// The tagged entity should pass through to the sub view delta.
	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);

	PopulateViewDeltaWithComponentAdded(Delta, View, OtherTaggedEntityId, ComponentData{ TagComponentId });
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);

	PopulateViewDeltaWithComponentUpdated(Delta, View, OtherTaggedEntityId, CreateTestComponentUpdate(ValueComponentId, CorrectValue));
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, OtherTaggedEntityId);

	return true;
}

SUBVIEW_TEST(GIVEN_Tagged_Incomplete_Entity_Which_Should_Be_Complete_WHEN_Refresh_Entity_THEN_Entity_Is_Complete)
{
	const Worker_EntityId TaggedEntityId = 2;
	const Worker_ComponentId TagComponentId = 1;

	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	bool IsFilterComplete = false;

	FFilterPredicate Filter = [&IsFilterComplete](const Worker_EntityId&, const EntityViewElement&) {
		return IsFilterComplete;
	};

	FSubView SubView(TagComponentId, Filter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);

	AddEntityToView(View, TaggedEntityId);

	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);

	IsFilterComplete = true;
	SubView.RefreshEntity(TaggedEntityId);
	Delta.Clear();
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);

	return true;
}
} // namespace SpatialGDK
