// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include <memory>

#include "EntityComponentTestUtils.h"
#include "SpatialView/Callbacks.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
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

TUniquePtr<SpatialGDK::ComponentChange> CreateComponentAddedChange(const Worker_ComponentId ComponentId, const double Value)
{
	SpatialGDK::ComponentData Data = SpatialGDK::CreateTestComponentData(ComponentId, Value);
	return MakeUnique<SpatialGDK::ComponentChange>(ComponentId, MoveTemp(Data).Release());
}

TArray<SpatialGDK::EntityDelta> CreateComponentAddedDeltaWithChange(const Worker_EntityId EntityId,
																	const TUniquePtr<SpatialGDK::ComponentChange>& Change)
{
	SpatialGDK::EntityDelta Delta = {};
	Delta.EntityId = EntityId;
	Delta.ComponentsAdded = SpatialGDK::ComponentSpan<SpatialGDK::ComponentChange>(Change.Get(), 1);
	return TArray<SpatialGDK::EntityDelta>{ Delta };
}

double GetValueFromSchemaComponentData(Schema_ComponentData* Data)
{
	return Schema_GetDouble(Schema_GetComponentDataFields(Data), SpatialGDK::EntityComponentTestUtils::TEST_DOUBLE_FIELD_ID);
}

SpatialGDK::EntityViewElement CreateViewElementWithComponentValue(const Worker_ComponentId ComponentId, const double Value)
{
	SpatialGDK::ComponentData Data = SpatialGDK::CreateTestComponentData(ComponentId, Value);
	SpatialGDK::EntityViewElement Element;
	Element.Components.Emplace(MoveTemp(Data));
	return Element;
}
} // anonymous namespace

//
// Tests begin here
//

DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_Then_Invoked_THEN_Callback_Invoked_With_Correct_Values)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange Change) {
		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
			&& GetValueFromSchemaComponentData(Change.Change.Data) == COMPONENT_VALUE)
		{
			Invoked = true;
		}
	};

	Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
	TUniquePtr<SpatialGDK::ComponentChange> Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(CreateComponentAddedDeltaWithChange(ENTITY_ID, Change));

	TestTrue("Callback was invoked", Invoked);

	// Now a few more times, but with incorrect values, just in case
	Invoked = false;

	Change = CreateComponentAddedChange(COMPONENT_ID, OTHER_COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(CreateComponentAddedDeltaWithChange(ENTITY_ID, Change));
	TestFalse("Callback was not invoked", Invoked);

	Change = CreateComponentAddedChange(OTHER_COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(CreateComponentAddedDeltaWithChange(ENTITY_ID, Change));
	TestFalse("Callback was not invoked", Invoked);

	Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(CreateComponentAddedDeltaWithChange(OTHER_ENTITY_ID, Change));
	TestFalse("Callback was not invoked", Invoked);

	return true;
}

DISPATCHER_TEST(GIVEN_Dispatcher_With_Callback_WHEN_Callback_Removed_THEN_Callback_Not_Invoked)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](SpatialGDK::FEntityComponentChange) {
		Invoked = true;
	};

	const SpatialGDK::CallbackId Id = Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
	const TUniquePtr<SpatialGDK::ComponentChange> Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
	const TArray<SpatialGDK::EntityDelta> Deltas = CreateComponentAddedDeltaWithChange(ENTITY_ID, Change);
	Dispatcher.InvokeCallbacks(Deltas);

	TestTrue("Callback was invoked", Invoked);

	Invoked = false;
	Dispatcher.RemoveCallback(Id);
	Dispatcher.InvokeCallbacks(Deltas);

	TestFalse("Callback was not invoked again", Invoked);

	return true;
}

DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_And_Invoked_THEN_Callback_Invoked_With_Correct_Values)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher;
	const TUniquePtr<SpatialGDK::EntityView> View = MakeUnique<SpatialGDK::EntityView>();

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](const SpatialGDK::FEntityComponentChange Change) {
		if (Change.EntityId == ENTITY_ID && Change.Change.ComponentId == COMPONENT_ID
			&& GetValueFromSchemaComponentData(Change.Change.Data) == COMPONENT_VALUE)
		{
			Invoked = true;
		}
	};

	View->Emplace(ENTITY_ID, CreateViewElementWithComponentValue(COMPONENT_ID, COMPONENT_VALUE));

	Dispatcher.RegisterAndInvokeComponentAddedCallback(COMPONENT_ID, Callback, View.Get());

	TestTrue("Callback was invoked", Invoked);

	// Double check the callback is actually called on invocation as well
	Invoked = false;
	const TUniquePtr<SpatialGDK::ComponentChange> Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
	const TArray<SpatialGDK::EntityDelta> Deltas = CreateComponentAddedDeltaWithChange(ENTITY_ID, Change);
	Dispatcher.InvokeCallbacks(Deltas);

	TestTrue("Callback was invoked", Invoked);

	return true;
}
