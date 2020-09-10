// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "SpatialView/ViewCoordinator.h"
#include "SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"
#include "Utils/ComponentFactory.h"

#define SUBVIEW_TEST(TestName) GDK_TEST(Core, SubView, TestName)

namespace SpatialGDK
{
const Worker_EntityId ENTITY_ID = 1;
const Worker_EntityId TAGGED_ENTITY_ID = 2;
const Worker_EntityId OTHER_TAGGED_ENTITY_ID = 3;
const Worker_ComponentId TAG_COMPONENT_ID = 1;
const Worker_ComponentId VALUE_COMPONENT_ID = 2;
const double CORRECT_VALUE = 1;
const double INCORRECT_VALUE = 2;

FFilterPredicate NoFilter = [](const Worker_EntityId&, const EntityViewElement&) {
	return true;
};
FComponentChangeRefreshPredicate NoComponentChangeRefreshPredicate = [](const FEntityComponentChange&) {
	return true;
};
TArray<FDispatcherRefreshCallback> NoRefreshCallbacks = TArray<FDispatcherRefreshCallback>{};

SUBVIEW_TEST(GIVEN_Set_Of_Components_WHEN_Entity_Tagged_THEN_Components_Contains_Tag)
{
	FDispatcher Dispatcher;
	EntityView View;
	const FSubView SubView(TAG_COMPONENT_ID, NoFilter, &View, Dispatcher, NoRefreshCallbacks);
	TArray<FWorkerComponentData> Components{ ComponentFactory::CreateEmptyComponentData(VALUE_COMPONENT_ID) };

	SubView.TagEntity(Components);

	TestTrue("Components contains tag component", Components.ContainsByPredicate([](const FWorkerComponentData& Data) {
		return Data.component_id == TAG_COMPONENT_ID;
	}));

	return true;
}

SUBVIEW_TEST(GIVEN_Query_WHEN_Query_Tagged_THEN_Query_Is_Tagged)
{
	FDispatcher Dispatcher;
	EntityView View;
	FSubView SubView(TAG_COMPONENT_ID, NoFilter, &View, Dispatcher, NoRefreshCallbacks);
	Query QueryToTag;
	QueryConstraint Constraint;
	Constraint.EntityIdConstraint = ENTITY_ID;
	QueryToTag.Constraint = Constraint;
	QueryToTag.ResultComponentIds = SchemaResultType{ VALUE_COMPONENT_ID };

	SubView.TagQuery(QueryToTag);

	if (!TestNotEqual("Constraint is now an AND", QueryToTag.Constraint.AndConstraint.Num(), 0))
	{
		return true;
	}
	TArray<QueryConstraint>& AndConstraint = QueryToTag.Constraint.AndConstraint;
	Worker_ComponentId TestConstraintId = AndConstraint[0].ComponentConstraint.IsSet() ? AndConstraint[0].ComponentConstraint.GetValue()
																					   : AndConstraint[1].ComponentConstraint.GetValue();
	TestEqual("Constraint contains tag component constraint", TestConstraintId, TAG_COMPONENT_ID);

	TestTrue("Result type contains tag", QueryToTag.ResultComponentIds.Contains(TAG_COMPONENT_ID));

	return true;
}

SUBVIEW_TEST(GIVEN_SubView_Without_Filter_WHEN_Tagged_Entity_Added_THEN_Delta_Contains_Entity)
{
	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	FSubView SubView(TAG_COMPONENT_ID, NoFilter, &View, Dispatcher, NoRefreshCallbacks);

	AddEntityToView(View, TAGGED_ENTITY_ID);
	PopulateViewDeltaWithComponentAdded(Delta, View, TAGGED_ENTITY_ID, TAG_COMPONENT_ID);

	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	// The tagged entity should pass through to the sub view delta.
	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TAGGED_ENTITY_ID);

	return true;
}

SUBVIEW_TEST(
	GIVEN_SubView_With_Filter_WHEN_Tagged_Entities_Added_THEN_Delta_Only_Contains_Filtered_Entities_ALSO_Dispatcher_Callback_Refreshes_Correctly)
{
	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	FSubView SubView( TAG_COMPONENT_ID,
				 [](const Worker_EntityId&, const EntityViewElement& Element) {
					 const ComponentData* It = Element.Components.FindByPredicate(ComponentIdEquality{ VALUE_COMPONENT_ID });
					 if (GetValueFromSchemaComponentData(It->GetUnderlying()) == CORRECT_VALUE)
					 {
						 return true;
					 }
					 return false;
				 },
				 &View, Dispatcher,
				 TArray<FDispatcherRefreshCallback>{
					 FSubView::CreateComponentChangedRefreshCallback(Dispatcher, VALUE_COMPONENT_ID, NoComponentChangeRefreshPredicate) } );

	AddEntityToView(View, TAGGED_ENTITY_ID);
	AddComponentToView(View, TAGGED_ENTITY_ID, CreateTestComponentData(VALUE_COMPONENT_ID, CORRECT_VALUE));
	AddEntityToView(View, OTHER_TAGGED_ENTITY_ID);
	AddComponentToView(View, OTHER_TAGGED_ENTITY_ID, CreateTestComponentData(VALUE_COMPONENT_ID, INCORRECT_VALUE));

	PopulateViewDeltaWithComponentAdded(Delta, View, TAGGED_ENTITY_ID, TAG_COMPONENT_ID);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	// The tagged entity should pass through to the sub view delta.
	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TAGGED_ENTITY_ID);

	PopulateViewDeltaWithComponentAdded(Delta, View, OTHER_TAGGED_ENTITY_ID, TAG_COMPONENT_ID);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);

	PopulateViewDeltaWithComponentUpdated(Delta, View, OTHER_TAGGED_ENTITY_ID, VALUE_COMPONENT_ID, CORRECT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, OTHER_TAGGED_ENTITY_ID);

	return true;
}

SUBVIEW_TEST(GIVEN_Tagged_Incomplete_Entity_Which_Should_Be_Complete_WHEN_Refresh_Entity_THEN_Entity_Is_Complete)
{
	FDispatcher Dispatcher;
	EntityView View;
	ViewDelta Delta;

	bool IsFilterComplete = false;

	FSubView SubView( TAG_COMPONENT_ID,
				 [&IsFilterComplete](const Worker_EntityId&, const EntityViewElement&) {
					 return IsFilterComplete;
				 },
				 &View, Dispatcher, NoRefreshCallbacks );

	AddEntityToView(View, TAGGED_ENTITY_ID);

	PopulateViewDeltaWithComponentAdded(Delta, View, TAGGED_ENTITY_ID, TAG_COMPONENT_ID);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	SubView.Advance(Delta);
	FSubViewDelta SubDelta = SubView.GetViewDelta();

	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);

	IsFilterComplete = true;
	SubView.RefreshEntity(TAGGED_ENTITY_ID);
	Delta.Clear();
	SubView.Advance(Delta);
	SubDelta = SubView.GetViewDelta();

	if (!TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1))
	{
		// early out so we don't crash - test has already failed
		return true;
	}
	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TAGGED_ENTITY_ID);

	return true;
}
} // namespace SpatialGDK
