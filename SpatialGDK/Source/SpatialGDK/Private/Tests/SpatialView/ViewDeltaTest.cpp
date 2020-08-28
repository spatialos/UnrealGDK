// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "OpsUtils.h"
#include "SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

#include "SpatialView/ViewDelta.h"

#define VIEWDELTA_TEST(TestName) GDK_TEST(Core, ViewDelta, TestName)

using namespace SpatialGDK;

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_remove_THEN_get_empty_view_and_delta)
{
	EntityView InputView;
	ViewDelta InputDelta;

	TArray<Worker_Op> InputOpList = {
		CreateAddEntityOp(2),
		CreateRemoveEntityOp(2),
	};
	InputDelta.SetFromOpList(ConstructOpList(InputOpList), InputView);

	EntityView ExpectedView;
	ViewDelta ExpectedDelta;

	TestTrue("View Deltas are equal", AreEquivalent(InputDelta, ExpectedDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_remove_add_THEN_entity_in_view_and_delta)
{
	EntityView InputView;
	ViewDelta InputDelta;

	TArray<Worker_Op> InputOpList = {
		CreateAddEntityOp(2),
		CreateRemoveEntityOp(2),
		CreateAddEntityOp(2),
	};
	InputDelta.SetFromOpList(ConstructOpList(InputOpList), InputView);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, 2);

	EntityDelta Delta = CreateEntityDelta(2, ADD);
	TArray<EntityDelta> Deltas = { Delta };
	ViewDelta ExpectedDelta = ViewDelta(Deltas);

	TestTrue("View Deltas are equal", AreEquivalent(InputDelta, ExpectedDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_empty_view_WHEN_add_entity_add_component_THEN_entity_and_component_in_view_and_delta)
{
	EntityView InputView;
	ViewDelta InputDelta;

	TArray<Worker_Op> InputOpList = {
		CreateAddEntityOp(2),
		CreateAddComponentOp(2, 1, 20),
	};
	InputDelta.SetFromOpList(ConstructOpList(InputOpList), InputView);

	TArray<ComponentChange> Changes;
	AddComponentAddedChange(Changes, 1, 20);
	EntityDelta Delta = CreateEntityDelta(2, ADD);
	Delta.ComponentsAdded = CreateTestComponentSpan(Changes);
	TArray<EntityDelta> Deltas = { Delta };
	ViewDelta ExpectedDelta = ViewDelta(Deltas);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, 2);
	AddComponentToView(ExpectedView, 2, 1, 20);

	TestTrue("View Deltas are equal", AreEquivalent(InputDelta, ExpectedDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));
	return true;
}

VIEWDELTA_TEST(GIVEN_entity_in_view__WHEN_add_component_THEN_view_has_component)
{
	EntityView InputView;
	AddEntityToView(InputView, 2);
	ViewDelta InputDelta;

	TArray<Worker_Op> InputOpList = { CreateAddComponentOp(2, 1, 20) };
	InputDelta.SetFromOpList(ConstructOpList(InputOpList), InputView);

	EntityView ExpectedView;
	AddEntityToView(ExpectedView, 2);
	AddComponentToView(ExpectedView, 2, 1, 20);

	TArray<ComponentChange> Changes;
	AddComponentAddedChange(Changes, 1, 20);
	EntityDelta Delta = CreateEntityDelta(2, UPDATE);
	Delta.ComponentsAdded = CreateTestComponentSpan(Changes);
	TArray<EntityDelta> Deltas = { Delta };
	ViewDelta ExpectedDelta = ViewDelta(Deltas);

	TestTrue("View Deltas are equal", AreEquivalent(InputDelta, ExpectedDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}

VIEWDELTA_TEST(GIVEN_entity_and_component_in_view_WHEN_remove_entity_THEN_empty_view_remove_ops_in_delta)
{
	EntityView InputView;
	AddEntityToView(InputView, 2);
	AddComponentToView(InputView, 2, 1, 20);

	ViewDelta InputDelta;
	TArray<Worker_Op> InputOpList = { CreateRemoveEntityOp(2) };
	InputDelta.SetFromOpList(ConstructOpList(InputOpList), InputView);

	EntityView ExpectedView;

	EntityDelta Delta = CreateEntityDelta(2, REMOVE);
	TArray<ComponentChange> RemovedComponents;
	AddComponentRemovedChange(RemovedComponents, 1);
	Delta.ComponentsRemoved = CreateTestComponentSpan(RemovedComponents);
	TArray<EntityDelta> Deltas = { Delta };
	ViewDelta ExpectedDelta = ViewDelta(Deltas);

	TestTrue("View Deltas are equal", AreEquivalent(InputDelta, ExpectedDelta));
	TestTrue("Views are equal", AreEquivalent(InputView, ExpectedView));

	return true;
}
