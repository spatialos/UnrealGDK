// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

using namespace SpatialGDK;

namespace
{
	const Worker_EntityId TestEntityId = 1337;

	const Worker_ComponentId TestComponentId = 1338;
	const Worker_ComponentId ComponentIdToRemove = 1347;
	const Worker_ComponentId ComponentIdToKeep = 1348;

	const int EventValue = 7332;

	const double TestValue = 7331;
	const double TestUpdateValue = 7332;
	const double UpdateValue = 7333;
}  // anonymous namespace

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_update_added_THEN_update_record_has_the_update)
{
	// GIVEN
	ComponentUpdate TestUpdate = CreateTestComponentUpdate(TestComponentId, TestValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TestEntityId, TestUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	ComponentUpdate FirstUpdate = CreateTestComponentUpdate(TestComponentId, TestValue);
	ComponentUpdate SecondUpdate = CreateTestComponentUpdate(TestComponentId, TestUpdateValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TestEntityId, SecondUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(FirstUpdate));

	// WHEN
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(SecondUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_complete_update_added_THEN_update_record_has_complete_update)
{
	// GIVEN
	ComponentData Data = CreateTestComponentData(TestComponentId, TestValue);

	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ TestEntityId, Data.DeepCopy(), ComponentUpdate(TestComponentId) });
	const TArray<EntityComponentUpdate> ExpectedUpdates = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentDataAsUpdate(TestEntityId, MoveTemp(Data));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_complete_update_added_THEN_complete_update_merged)
{
	// GIVEN
	ComponentUpdate Update = CreateTestComponentUpdate(TestComponentId, TestValue);
	AddTestEvent(&Update, EventValue);
	ComponentData CompleteUpdate = CreateTestComponentData(TestComponentId, UpdateValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push({ TestEntityId, CompleteUpdate.DeepCopy(), Update.DeepCopy() });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(Update));

	// WHEN
	Storage.AddComponentDataAsUpdate(TestEntityId, MoveTemp(CompleteUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_a_complete_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	ComponentData CompleteUpdate = CreateTestComponentData(TestComponentId, TestValue);
	ComponentUpdate Update = CreateTestComponentUpdate(TestComponentId, UpdateValue);
	AddTestEvent(&Update, EventValue);
	ComponentUpdate AdditionalEvent = CreateTestComponentEvent(TestComponentId, EventValue);

	ComponentData ExpectedCompleteUpdate = CreateTestComponentData(TestComponentId, UpdateValue);
	ComponentUpdate ExpectedEvent = CreateTestComponentEvent(TestComponentId, EventValue);
	AddTestEvent(&ExpectedEvent, EventValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates{};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ TestEntityId, MoveTemp(ExpectedCompleteUpdate), MoveTemp(ExpectedEvent) });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(TestEntityId, MoveTemp(CompleteUpdate));

	// WHEN
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(Update));
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(AdditionalEvent));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_multiple_updates_WHEN_component_removed_THEN_its_updates_removed)
{
	// GIVEN
	ComponentData CompleteUpdateToRemove = CreateTestComponentData(ComponentIdToRemove, TestValue);
	ComponentUpdate EventToRemove = CreateTestComponentEvent(ComponentIdToRemove, EventValue);

	ComponentUpdate UpdateToKeep = CreateTestComponentUpdate(ComponentIdToKeep, UpdateValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ TestEntityId, UpdateToKeep.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(TestEntityId, MoveTemp(CompleteUpdateToRemove));
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(EventToRemove));
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(UpdateToKeep));

	// WHEN
	Storage.RemoveComponent(TestEntityId, ComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_component_removed_THEN_its_update_removed)
{
	// GIVEN
	ComponentUpdate Update = CreateTestComponentUpdate(ComponentIdToRemove, UpdateValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(TestEntityId, MoveTemp(Update));

	// WHEN
	Storage.RemoveComponent(TestEntityId, ComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}
