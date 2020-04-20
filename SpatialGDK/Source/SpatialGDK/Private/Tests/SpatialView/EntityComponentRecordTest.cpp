// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTRECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentRecord, TestName)

using namespace SpatialGDK;

namespace
{
	const Worker_EntityId TestEntityId = 1337;
	const Worker_ComponentId TestComponentId = 1338;
	const double TestValue = 7331;
	const double TestUpdateValue = 7332;
}  // anonymous namespace

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_added_THEN_has_component_data)
{
	// GIVEN
	auto TestData = CreateTestComponentData(TestComponentId, TestValue);

	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};
	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TestEntityId, TestData.DeepCopy() });

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponent(TestEntityId, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_removed_THEN_has_removed_component_id)
{
	// GIVEN
	const EntityComponentId kEntityComponentId = { TestEntityId, TestComponentId };
	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = { kEntityComponentId };

	EntityComponentRecord Storage;

	// WHEN
	Storage.RemoveComponent(kEntityComponentId.EntityId, kEntityComponentId.ComponentId);

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));

	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_that_component_removed_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TestComponentId, TestValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.RemoveComponent(TestEntityId, TestComponentId);

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

// This should not produce the component added - just cancel removing it
ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_removed_component_WHEN_component_added_again_THEN_component_record_is_empty)
{
	// GIVEN
	auto TestData = CreateTestComponentData(TestComponentId, TestValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.RemoveComponent(TestEntityId, TestComponentId);

	// WHEN
	Storage.AddComponent(TestEntityId, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TestComponentId, TestValue);
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TestComponentId, TestUpdateValue);
	ComponentData ExpectedData = CreateTestComponentData(TestComponentId, TestUpdateValue);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TestEntityId, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.AddUpdate(TestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_updated_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TestComponentId, TestUpdateValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddUpdate(TestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_complete_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TestComponentId, TestValue);
	ComponentData TestUpdate = CreateTestComponentData(TestComponentId, TestUpdateValue);
	ComponentData ExpectedData = CreateTestComponentData(TestComponentId, TestUpdateValue);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TestEntityId, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.AddComponentAsUpdate(TestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_complete_update_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentData TestUpdate = CreateTestComponentData(TestComponentId, TestUpdateValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponentAsUpdate(TestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

