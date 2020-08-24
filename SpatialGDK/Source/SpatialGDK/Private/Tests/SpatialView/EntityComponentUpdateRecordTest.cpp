// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

namespace SpatialGDK
{
namespace
{
const Worker_EntityId TEST_ENTITY_ID = 1337;

const Worker_ComponentId TEST_COMPONENT_ID = 1338;
const Worker_ComponentId COMPONENT_ID_TO_REMOVE = 1347;
const Worker_ComponentId COMPONENT_ID_TO_KEEP = 1348;

const int EVENT_VALUE = 7332;

const double TEST_VALUE = 7331;
const double TEST_UPDATE_VALUE = 7332;
const double UPDATE_VALUE = 7333;
} // anonymous namespace

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_update_added_THEN_update_record_has_the_update)
{
	// GIVEN
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_VALUE);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TEST_ENTITY_ID, TestUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	ComponentUpdate FirstUpdate = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_VALUE);
	ComponentUpdate SecondUpdate = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_UPDATE_VALUE);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TEST_ENTITY_ID, SecondUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(FirstUpdate));

	// WHEN
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(SecondUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_complete_update_added_THEN_update_record_has_complete_update)
{
	// GIVEN
	ComponentData Data = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);

	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ TEST_ENTITY_ID, Data.DeepCopy(), ComponentUpdate(TEST_COMPONENT_ID) });
	const TArray<EntityComponentUpdate> ExpectedUpdates = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentDataAsUpdate(TEST_ENTITY_ID, MoveTemp(Data));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_complete_update_added_THEN_complete_update_merged)
{
	// GIVEN
	ComponentUpdate Update = CreateTestComponentUpdate(TEST_COMPONENT_ID, TEST_VALUE);
	AddTestEvent(&Update, EVENT_VALUE);
	ComponentData CompleteUpdate = CreateTestComponentData(TEST_COMPONENT_ID, UPDATE_VALUE);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push({ TEST_ENTITY_ID, CompleteUpdate.DeepCopy(), Update.DeepCopy() });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(Update));

	// WHEN
	Storage.AddComponentDataAsUpdate(TEST_ENTITY_ID, MoveTemp(CompleteUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_a_complete_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	ComponentData CompleteUpdate = CreateTestComponentData(TEST_COMPONENT_ID, TEST_VALUE);
	ComponentUpdate Update = CreateTestComponentUpdate(TEST_COMPONENT_ID, UPDATE_VALUE);
	AddTestEvent(&Update, EVENT_VALUE);
	ComponentUpdate AdditionalEvent = CreateTestComponentEvent(TEST_COMPONENT_ID, EVENT_VALUE);

	ComponentData ExpectedCompleteUpdate = CreateTestComponentData(TEST_COMPONENT_ID, UPDATE_VALUE);
	ComponentUpdate ExpectedEvent = CreateTestComponentEvent(TEST_COMPONENT_ID, EVENT_VALUE);
	AddTestEvent(&ExpectedEvent, EVENT_VALUE);

	const TArray<EntityComponentUpdate> ExpectedUpdates{};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(
		EntityComponentCompleteUpdate{ TEST_ENTITY_ID, MoveTemp(ExpectedCompleteUpdate), MoveTemp(ExpectedEvent) });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(TEST_ENTITY_ID, MoveTemp(CompleteUpdate));

	// WHEN
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(Update));
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(AdditionalEvent));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_multiple_updates_WHEN_component_removed_THEN_its_updates_removed)
{
	// GIVEN
	ComponentData CompleteUpdateToRemove = CreateTestComponentData(COMPONENT_ID_TO_REMOVE, TEST_VALUE);
	ComponentUpdate EventToRemove = CreateTestComponentEvent(COMPONENT_ID_TO_REMOVE, EVENT_VALUE);

	ComponentUpdate UpdateToKeep = CreateTestComponentUpdate(COMPONENT_ID_TO_KEEP, UPDATE_VALUE);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TEST_ENTITY_ID, UpdateToKeep.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(TEST_ENTITY_ID, MoveTemp(CompleteUpdateToRemove));
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(EventToRemove));
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(UpdateToKeep));

	// WHEN
	Storage.RemoveComponent(TEST_ENTITY_ID, COMPONENT_ID_TO_REMOVE);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_component_removed_THEN_its_update_removed)
{
	// GIVEN
	ComponentUpdate Update = CreateTestComponentUpdate(COMPONENT_ID_TO_REMOVE, UPDATE_VALUE);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TEST_ENTITY_ID, MoveTemp(Update));

	// WHEN
	Storage.RemoveComponent(TEST_ENTITY_ID, COMPONENT_ID_TO_REMOVE);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

} // namespace SpatialGDK
