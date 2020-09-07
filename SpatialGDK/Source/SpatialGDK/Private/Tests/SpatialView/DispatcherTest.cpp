// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include <memory>

#include "ComponentTestUtils.h"
#include "SpatialViewUtils.h"
#include "SpatialView/Callbacks.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/ViewDelta.h"
#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/TestDefinitions.h"

#define DISPATCHER_TEST(TestName) GDK_TEST(Core, Dispatcher, TestName)

namespace
{
constexpr Worker_ComponentId COMPONENT_ID = 1000;
constexpr Worker_ComponentId OTHER_COMPONENT_ID = 1001;
constexpr Worker_EntityId ENTITY_ID = 1;
constexpr Worker_EntityId OTHER_ENTITY_ID = 2;
constexpr double COMPONENT_VALUE = 3;
constexpr double OTHER_COMPONENT_VALUE = 4;

void PopulateViewDeltaWithComponentAdded(SpatialGDK::ViewDelta& Delta, SpatialGDK::EntityView& View, const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, const double Value)
{
	SpatialGDK::EntityComponentOpListBuilder OpListBuilder = SpatialGDK::EntityComponentOpListBuilder{};
	OpListBuilder.AddComponent(EntityId, SpatialGDK::CreateTestComponentData(ComponentId, Value));
	SetFromOpList(Delta, View, OpListBuilder);
}

double GetValueFromSchemaComponentData(Schema_ComponentData* Data)
{
	return Schema_GetDouble(Schema_GetComponentDataFields(Data), SpatialGDK::EntityComponentTestUtils::TEST_DOUBLE_FIELD_ID);
}
} // anonymous namespace

DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_Then_Invoked_THEN_Callback_Invoked_With_Correct_Values)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange Change) {
		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
			&& GetValueFromSchemaComponentData(Change.Change.Data) == COMPONENT_VALUE)
		{
			Invoked = true;
		}
	};
	Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);

	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	TestTrue("Callback was invoked", Invoked);

	// Now a few more times, but with incorrect values, just in case
	Invoked = false;

	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, COMPONENT_ID, OTHER_COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	TestFalse("Callback was not invoked", Invoked);

	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, OTHER_COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	TestFalse("Callback was not invoked", Invoked);

	PopulateViewDeltaWithComponentAdded(Delta, View, OTHER_ENTITY_ID, COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());
	TestFalse("Callback was not invoked", Invoked);

	return true;
}

DISPATCHER_TEST(GIVEN_Dispatcher_With_Callback_WHEN_Callback_Removed_THEN_Callback_Not_Invoked)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](SpatialGDK::FEntityComponentChange) {
		Invoked = true;
	};

	const SpatialGDK::CallbackId Id = Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	TestTrue("Callback was invoked", Invoked);

	Invoked = false;
	Dispatcher.RemoveCallback(Id);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	TestFalse("Callback was not invoked again", Invoked);

	return true;
}

DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_And_Invoked_THEN_Callback_Invoked_With_Correct_Values)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;
	SpatialGDK::EntityView View;
	SpatialGDK::ViewDelta Delta;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange Change) {
		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
			&& GetValueFromSchemaComponentData(Change.Change.Data) == COMPONENT_VALUE)
		{
			Invoked = true;
		}
	};

	AddEntityToView(View, ENTITY_ID);
	AddComponentToView(View, ENTITY_ID, SpatialGDK::CreateTestComponentData(COMPONENT_ID, COMPONENT_VALUE));

	Dispatcher.RegisterAndInvokeComponentAddedCallback(COMPONENT_ID, Callback, View);

	TestTrue("Callback was invoked", Invoked);

	// Double check the callback is actually called on invocation as well.
	Invoked = false;
	PopulateViewDeltaWithComponentAdded(Delta, View, ENTITY_ID, COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(Delta.GetEntityDeltas());

	TestTrue("Callback was invoked", Invoked);

	return true;
}
