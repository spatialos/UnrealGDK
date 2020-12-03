// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "SpatialView/OpList/EntityComponentOpList.h"
// #include "SpatialView/ViewCoordinator.h"
// #include "Tests/SpatialView/SpatialViewUtils.h"
// #include "Tests/TestDefinitions.h"
// #include "Utils/ComponentFactory.h"
//
// #define SUBVIEW_TEST(TestName) GDK_TEST(Core, SubView, TestName)
//
// namespace SpatialGDK
// {
// SUBVIEW_TEST(GIVEN_SubView_Without_Filter_WHEN_Tagged_Entity_Added_THEN_Delta_Contains_Entity)
// {
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_ComponentId TagComponentId = 1;
//
// 	FDispatcher Dispatcher;
// 	EntityView View;
// 	ViewDelta Delta;
//
// 	FSubView SubView(TagComponentId, FSubView::NoFilter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);
//
// 	AddEntityToView(View, TaggedEntityId);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });
//
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	SubView.Advance(Delta);
// 	FSubViewDelta SubDelta = SubView.GetViewDelta();
//
// 	// The tagged entity should pass through to the sub view delta.
// 	TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1);
// 	if (SubDelta.EntityDeltas.Num() != 1)
// 	{
// 		// early out so we don't crash - test has already failed
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);
//
// 	return true;
// }
//
// SUBVIEW_TEST(
// 	GIVEN_SubView_With_Filter_WHEN_Tagged_Entities_Added_THEN_Delta_Only_Contains_Filtered_Entities_ALSO_Dispatcher_Callback_Refreshes_Correctly)
// {
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_EntityId OtherTaggedEntityId = 3;
// 	const Worker_ComponentId TagComponentId = 1;
// 	const Worker_ComponentId ValueComponentId = 2;
// 	const double CorrectValue = 1;
// 	const double IncorrectValue = 2;
//
// 	FDispatcher Dispatcher;
// 	EntityView View;
// 	ViewDelta Delta;
//
// 	FFilterPredicate Filter = [ValueComponentId, CorrectValue](const Worker_EntityId&, const EntityViewElement& Element) {
// 		const ComponentData* It = Element.Components.FindByPredicate(ComponentIdEquality{ ValueComponentId });
// 		if (GetValueFromTestComponentData(It->GetUnderlying()) == CorrectValue)
// 		{
// 			return true;
// 		}
// 		return false;
// 	};
// 	auto RefreshCallbacks = TArray<FDispatcherRefreshCallback>{ FSubView::CreateComponentChangedRefreshCallback(
// 		Dispatcher, ValueComponentId, FSubView::NoComponentChangeRefreshPredicate) };
//
// 	FSubView SubView(TagComponentId, Filter, &View, Dispatcher, RefreshCallbacks);
//
// 	AddEntityToView(View, TaggedEntityId);
// 	AddComponentToView(View, TaggedEntityId, CreateTestComponentData(ValueComponentId, CorrectValue));
// 	AddEntityToView(View, OtherTaggedEntityId);
// 	AddComponentToView(View, OtherTaggedEntityId, CreateTestComponentData(ValueComponentId, IncorrectValue));
//
// 	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	SubView.Advance(Delta);
// 	FSubViewDelta SubDelta = SubView.GetViewDelta();
//
// 	// The tagged entity should pass through to the sub view delta.
// 	TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1);
// 	if (SubDelta.EntityDeltas.Num() != 1)
// 	{
// 		// early out so we don't crash - test has already failed
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);
//
// 	PopulateViewDeltaWithComponentAdded(Delta, View, OtherTaggedEntityId, ComponentData{ TagComponentId });
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	SubView.Advance(Delta);
// 	SubDelta = SubView.GetViewDelta();
//
// 	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);
//
// 	PopulateViewDeltaWithComponentUpdated(Delta, View, OtherTaggedEntityId, CreateTestComponentUpdate(ValueComponentId, CorrectValue));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	SubView.Advance(Delta);
// 	SubDelta = SubView.GetViewDelta();
//
// 	TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1);
// 	if (SubDelta.EntityDeltas.Num() != 1)
// 	{
// 		// early out so we don't crash - test has already failed
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, OtherTaggedEntityId);
//
// 	return true;
// }
//
// SUBVIEW_TEST(GIVEN_Tagged_Incomplete_Entity_Which_Should_Be_Complete_WHEN_Refresh_Entity_THEN_Entity_Is_Complete)
// {
// 	const Worker_EntityId TaggedEntityId = 2;
// 	const Worker_ComponentId TagComponentId = 1;
//
// 	FDispatcher Dispatcher;
// 	EntityView View;
// 	ViewDelta Delta;
//
// 	bool IsFilterComplete = false;
//
// 	FFilterPredicate Filter = [&IsFilterComplete](const Worker_EntityId&, const EntityViewElement&) {
// 		return IsFilterComplete;
// 	};
//
// 	FSubView SubView(TagComponentId, Filter, &View, Dispatcher, FSubView::NoDispatcherCallbacks);
//
// 	AddEntityToView(View, TaggedEntityId);
//
// 	PopulateViewDeltaWithComponentAdded(Delta, View, TaggedEntityId, ComponentData{ TagComponentId });
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	SubView.Advance(Delta);
// 	FSubViewDelta SubDelta = SubView.GetViewDelta();
//
// 	TestEqual("There are no entity deltas", SubDelta.EntityDeltas.Num(), 0);
//
// 	IsFilterComplete = true;
// 	SubView.RefreshEntity(TaggedEntityId);
// 	Delta.Clear();
// 	SubView.Advance(Delta);
// 	SubDelta = SubView.GetViewDelta();
//
// 	TestEqual("There is one entity delta", SubDelta.EntityDeltas.Num(), 1);
// 	if (SubDelta.EntityDeltas.Num() != 1)
// 	{
// 		// early out so we don't crash - test has already failed
// 		return true;
// 	}
// 	TestEqual("The entity delta is for the correct entity ID", SubDelta.EntityDeltas[0].EntityId, TaggedEntityId);
//
// 	return true;
// }
// } // namespace SpatialGDK
