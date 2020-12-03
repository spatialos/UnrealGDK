// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved
//
// #include "SpatialView/Callbacks.h"
// #include "SpatialView/ComponentData.h"
// #include "SpatialView/Dispatcher.h"
// #include "SpatialView/EntityDelta.h"
// #include "SpatialView/EntityView.h"
// #include "SpatialView/OpList/EntityComponentOpList.h"
// #include "SpatialView/ViewDelta.h"
// #include "Tests/SpatialView/ComponentTestUtils.h"
// #include "Tests/SpatialView/SpatialViewUtils.h"
// #include "Tests/TestDefinitions.h"
//
// #define DISPATCHER_TEST(TestName) GDK_TEST(Core, Dispatcher, TestName)
//
// namespace
// {
// constexpr Worker_ComponentId COMPONENT_ID = 1000;
// constexpr Worker_ComponentId OTHER_COMPONENT_ID = 1001;
// constexpr Worker_EntityId ENTITY_ID = 1;
// constexpr Worker_EntityId OTHER_ENTITY_ID = 2;
// constexpr double COMPONENT_VALUE = 3;
// constexpr double OTHER_COMPONENT_VALUE = 4;
// } // anonymous namespace
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_Then_Invoked_THEN_Callback_Invoked_With_Correct_Values)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange& Change) {
// 		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
// 			&& SpatialGDK::GetValueFromTestComponentData(Change.Change.Data) == COMPONENT_VALUE)
// 		{
// 			Invoked = true;
// 		}
// 	};
// 	Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	// Now a few more times, but with incorrect values, just in case
// 	Invoked = false;
//
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, OTHER_COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	TestFalse("Callback was not invoked", Invoked);
//
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(OTHER_COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	TestFalse("Callback was not invoked", Invoked);
//
// 	AddEntityToView(View, OTHER_ENTITY_ID);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, OTHER_ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
// 	TestFalse("Callback was not invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_With_Callback_WHEN_Callback_Removed_THEN_Callback_Not_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange&) {
// 		Invoked = true;
// 	};
//
// 	const SpatialGDK::CallbackId Id = Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
// 	AddEntityToView(View, ENTITY_ID);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	Invoked = false;
// 	Dispatcher.RemoveCallback(Id);
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestFalse("Callback was not invoked again", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_And_Invoked_THEN_Callback_Invoked_With_Correct_Values)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange& Change) {
// 		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
// 			&& SpatialGDK::GetValueFromTestComponentData(Change.Change.Data) == COMPONENT_VALUE)
// 		{
// 			Invoked = true;
// 		}
// 	};
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
//
// 	Dispatcher.RegisterAndInvokeComponentAddedCallback(COMPONENT_ID, Callback, View);
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	// Double check the callback is actually called on invocation as well.
// 	View[ENTITY_ID].Components.Empty();
// 	Invoked = false;
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Component_Changed_Callback_Added_Then_Invoked_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterComponentValueCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	PopulateViewDeltaWithComponentUpdated(Delta, View, ENTITY_ID,
// 										  SpatialGDK::CreateTestComponentUpdate(COMPONENT_ID, OTHER_COMPONENT_VALUE));
// 	Invoked = false;
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked again", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Component_Removed_Callback_Added_Then_Invoked_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterComponentRemovedCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
//
// 	PopulateViewDeltaWithComponentRemoved(Delta, View, ENTITY_ID, COMPONENT_ID);
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Authority_Gained_Callback_Added_Then_Invoked_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FEntityCallback Callback = [&Invoked](const Worker_EntityId&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterAuthorityGainedCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
//
// 	PopulateViewDeltaWithAuthorityChange(Delta, View, ENTITY_ID, COMPONENT_ID, WORKER_AUTHORITY_AUTHORITATIVE);
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Authority_Lost_Callback_Added_Then_Invoked_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FEntityCallback Callback = [&Invoked](const Worker_EntityId&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterAuthorityLostCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
// 	AddAuthorityToView(View, ENTITY_ID, COMPONENT_ID);
//
// 	PopulateViewDeltaWithAuthorityChange(Delta, View, ENTITY_ID, COMPONENT_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Authority_Lost_Temp_Callback_Added_Then_Invoked_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FEntityCallback Callback = [&Invoked](const Worker_EntityId&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterAuthorityLostTempCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
// 	AddAuthorityToView(View, ENTITY_ID, COMPONENT_ID);
//
// 	PopulateViewDeltaWithAuthorityLostTemp(Delta, View, ENTITY_ID, COMPONENT_ID);
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Many_Callbacks_Added_Then_Invoked_THEN_All_Callbacks_Correctly_Invoked)
// {
// 	int InvokeCount = 0;
// 	int NumberOfCallbacks = 100;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&InvokeCount](const SpatialGDK::FEntityComponentChange&) {
// 		++InvokeCount;
// 	};
// 	for (int i = 0; i < NumberOfCallbacks; ++i)
// 	{
// 		Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
// 	}
//
// 	AddEntityToView(View, ENTITY_ID);
// 	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestEqual("Callback was invoked the expected number of times", InvokeCount, NumberOfCallbacks);
//
// 	return true;
// }
//
// DISPATCHER_TEST(GIVEN_Dispatcher_With_Component_Removed_Callback_WHEN_Entity_Removed_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterComponentRemovedCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
//
// 	SpatialGDK::EntityComponentOpListBuilder OpListBuilder;
// 	OpListBuilder.RemoveComponent(ENTITY_ID, COMPONENT_ID);
// 	OpListBuilder.RemoveEntity(ENTITY_ID);
//
// 	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
//
// DISPATCHER_TEST(
// 	GIVEN_Dispatcher_With_Component_Removed_Callback_WHEN_Entity_Removed_And_Added_With_Different_Components_THEN_Callback_Invoked)
// {
// 	bool Invoked = false;
// 	SpatialGDK::FDispatcher Dispatcher;
// 	SpatialGDK::EntityView View;
// 	SpatialGDK::ViewDelta Delta;
//
// 	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange&) {
// 		Invoked = true;
// 	};
// 	Dispatcher.RegisterComponentRemovedCallback(COMPONENT_ID, Callback);
//
// 	AddEntityToView(View, ENTITY_ID);
// 	AddComponentToView(View, ENTITY_ID, SpatialGDK::ComponentData{ COMPONENT_ID });
//
// 	SpatialGDK::EntityComponentOpListBuilder OpListBuilder;
// 	OpListBuilder.RemoveComponent(ENTITY_ID, COMPONENT_ID);
// 	OpListBuilder.RemoveEntity(ENTITY_ID);
// 	OpListBuilder.AddEntity(ENTITY_ID);
// 	OpListBuilder.AddComponent(ENTITY_ID, SpatialGDK::ComponentData{ OTHER_COMPONENT_ID });
//
// 	SetFromOpList(Delta, View, MoveTemp(OpListBuilder));
// 	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
//
// 	TestTrue("Callback was invoked", Invoked);
//
// 	return true;
// }
