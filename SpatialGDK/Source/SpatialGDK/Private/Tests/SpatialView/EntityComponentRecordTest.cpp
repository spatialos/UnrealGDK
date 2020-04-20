// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTRECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentRecord, TestName)

using namespace SpatialGDK;

namespace
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const double kTestUpdateValue = 7332;
}  // anonymous namespace

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_added_THEN_has_component_data)
{
	// GIVEN
	auto TestData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};
	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, TestData.DeepCopy() });

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponent(kTestEntityId, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_component_removed_THEN_has_removed_component_id)
{
	// GIVEN
	const EntityComponentId kEntityComponentId = { kTestEntityId, kTestComponentId };
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
	ComponentData TestData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(kTestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.RemoveComponent(kTestEntityId, kTestComponentId);

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

// This should not produce the component added - just cancel removing it
ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_removed_component_WHEN_component_added_again_THEN_component_record_is_empty)
{
	// GIVEN
	auto TestData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.RemoveComponent(kTestEntityId, kTestComponentId);

	// WHEN
	Storage.AddComponent(kTestEntityId, MoveTemp(TestData));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(kTestComponentId, kTestValue);
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);
	ComponentData ExpectedData = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(kTestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.AddUpdate(kTestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_updated_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddUpdate(kTestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_component_record_with_component_WHEN_complete_update_added_THEN_component_record_has_updated_component_data)
{
	// GIVEN
	ComponentData TestData = CreateTestComponentData(kTestComponentId, kTestValue);
	ComponentData TestUpdate = CreateTestComponentData(kTestComponentId, kTestUpdateValue);
	ComponentData ExpectedData = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentData> ExpectedComponentsAdded;
	ExpectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, MoveTemp(ExpectedData) });
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;
	Storage.AddComponent(kTestEntityId, MoveTemp(TestData));

	// WHEN
	Storage.AddComponentAsUpdate(kTestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(GIVEN_empty_component_record_WHEN_complete_update_added_THEN_component_record_is_empty)
{
	// GIVEN
	ComponentData TestUpdate = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	const TArray<EntityComponentData> ExpectedComponentsAdded = {};
	const TArray<EntityComponentId> ExpectedComponentsRemoved = {};

	EntityComponentRecord Storage;

	// WHEN
	Storage.AddComponentAsUpdate(kTestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("ComponentsAdded are equal to expected"), AreEquivalent(Storage.GetComponentsAdded(), ExpectedComponentsAdded));
	TestTrue(TEXT("ComponentsRemoved are equal to expected"), AreEquivalent(Storage.GetComponentsRemoved(), ExpectedComponentsRemoved));
	return true;
}

