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
	constexpr Worker_EntityId ENTITY_ID = 1;
	constexpr double COMPONENT_VALUE = 2;

	struct ViewDispatcher
	{
		TUniquePtr<SpatialGDK::EntityView> View;
		SpatialGDK::FDispatcher Dispatcher;

		ViewDispatcher(TUniquePtr<SpatialGDK::EntityView>& View, SpatialGDK::FDispatcher Dispatcher)
		: View(MoveTemp(View)), Dispatcher(Dispatcher) {}
	};

	ViewDispatcher CreateViewAndDispatcher()
	{
		auto View = MakeUnique<SpatialGDK::EntityView>();
		SpatialGDK::FDispatcher Dispatcher(View.Get());
		return ViewDispatcher(View, Dispatcher);
	}

	TUniquePtr<SpatialGDK::ComponentChange> CreateComponentAddedChange(const Worker_ComponentId ComponentId,
		const double Value)
	{
		SpatialGDK::ComponentData Data = SpatialGDK::CreateTestComponentData(ComponentId, Value);
		return MakeUnique<SpatialGDK::ComponentChange>(ComponentId, MoveTemp(Data).Release());
	}

	TArray<SpatialGDK::EntityDelta> CreateComponentAddedDeltaWithChange(const Worker_EntityId EntityId, const TUniquePtr<SpatialGDK::ComponentChange>& Change)
	{
		SpatialGDK::EntityDelta Delta = {};
		Delta.EntityId = EntityId;
		Delta.ComponentsAdded = SpatialGDK::ComponentSpan<SpatialGDK::ComponentChange>(Change.Get(), 1);
		return TArray<SpatialGDK::EntityDelta>{ Delta };
	}

	void AddComponentToViewWithValue(TUniquePtr<SpatialGDK::EntityView>& View, const Worker_EntityId EntityId,
		const Worker_ComponentId ComponentId, const double Value)
	{
		SpatialGDK::ComponentData Data = SpatialGDK::CreateTestComponentData(ComponentId, Value);
		auto *Element = &View->Emplace(EntityId, SpatialGDK::EntityViewElement{});
		Element->Components.Emplace(MoveTemp(Data));
	}
} // anonymous namespace

DISPATCHER_TEST(GIVEN_Dispatcher_WHEN_Callback_Added_Then_Invoked_THEN_Callback_Invoked_With_Correct_Values)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher = CreateViewAndDispatcher().Dispatcher;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](SpatialGDK::FEntityComponentChange Change)
	{
		if (Change.EntityId == ENTITY_ID)
		{
			Invoked = true;
		}
	};

	Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
	TUniquePtr<SpatialGDK::ComponentChange> Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
	Dispatcher.InvokeCallbacks(CreateComponentAddedDeltaWithChange(ENTITY_ID, Change));

	TestTrue("Callback was invoked", Invoked);

	return true;
}

DISPATCHER_TEST(GIVEN_Dispatcher_With_Callback_WHEN_Callback_Removed_THEN_Callback_Not_Invoked)
{
	bool Invoked = false;
	SpatialGDK::FDispatcher Dispatcher = CreateViewAndDispatcher().Dispatcher;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](SpatialGDK::FEntityComponentChange)
	{
		Invoked = true;
	};

	const SpatialGDK::CallbackId Id = Dispatcher.RegisterComponentAddedCallback(COMPONENT_ID, Callback);
	TUniquePtr<SpatialGDK::ComponentChange> Change = CreateComponentAddedChange(COMPONENT_ID, COMPONENT_VALUE);
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
	ViewDispatcher ViewAndDispatcher = CreateViewAndDispatcher();
	SpatialGDK::FDispatcher Dispatcher = ViewAndDispatcher.Dispatcher;
	TUniquePtr<SpatialGDK::EntityView>& View = ViewAndDispatcher.View;

	const SpatialGDK::FComponentValueCallback Callback = [&Invoked](SpatialGDK::FEntityComponentChange Change)
	{
		if (Change.EntityId == ENTITY_ID)
		{
			Invoked = true;
		}
	};

	AddComponentToViewWithValue(View, ENTITY_ID, COMPONENT_ID, 2);

	Dispatcher.RegisterAndInvokeComponentAddedCallback(COMPONENT_ID, Callback);

	TestTrue("Callback was invoked", Invoked);

	return true;
}
