// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTRECORD_TEST(TestName) GDK_TEST(Core, EntityComponentRecord, TestName)

namespace SpatialGDK
{
namespace
{
const Worker_EntityId TEST_ENTITY_ID = 1337;
const Worker_ComponentId TEST_COMPONENT_ID = 1338;
const double TEST_VALUE = 7331;
const double TEST_UPDATE_VALUE = 7332;
} // anonymous namespace

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_added_THEN_has_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);

	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};
	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TEST_ENTITY_ID, TestData.DeepCopy() });

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponent(TEST_ENTITY_ID, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_removed_THEN_has_removed_component_id)
{
	// GIVEN
	const EntityComponentId kEntityComponentId = { TEST_ENTITY_ID, TEST_COMPONENT_ID };
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
	ComponentData TestData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TEST_ENTITY_ID, MoveTemp(TestData));

	// WHEN
	Storage.RemoveComponent(TEST_ENTITY_ID, TEST_COMPONENT_ID);

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

// This should not produce the component added - just cancel removing it
ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_removed_component_WHEN_component_added_again_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.RemoveComponent(TEST_ENTITY_ID, TEST_COMPONENT_ID);

	// WHEN
	Storage.AddComponent(TEST_ENTITY_ID, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);
	ComponentData ExpectedData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TEST_ENTITY_ID, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TEST_ENTITY_ID, MoveTemp(TestData));

	// WHEN
	Storage.AddUpdate(TEST_ENTITY_ID, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_updated_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddUpdate(TEST_ENTITY_ID, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(
	GIVEN_component_record_with_component_WHEN_complete_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);
	ComponentData TestUpdate = CreateTestComponentData(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);
	ComponentData ExpectedData = CreateTestComponentData(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ TEST_ENTITY_ID, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(TEST_ENTITY_ID, MoveTemp(TestData));

	// WHEN
	Storage.AddComponentAsUpdate(TEST_ENTITY_ID, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_complete_update_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentData TestUpdate = CreateTestComponentData(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponentAsUpdate(TEST_ENTITY_ID, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

} // namespace SpatialGDK
