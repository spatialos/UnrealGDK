// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

using namespace SpatialGDK;

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_update_added_THEN_update_record_has_the_update)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto TestUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, TestUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> ExpectedEvents = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(TestUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const double kTestUpdateValue = 7332;

	auto FirstUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);
	auto SecondUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, SecondUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> ExpectedEvents = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(FirstUpdate));

	// WHEN
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(SecondUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_complete_update_added_THEN_update_record_has_complete_update)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto Data = CreateTestComponentData(kTestComponentId, kTestValue);

	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ kTestEntityId, Data.DeepCopy(), ComponentUpdate(kTestComponentId) });
	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	const TArray<EntityComponentUpdate> ExpectedEvents = {};

	EntityComponentUpdateRecord Storage;

	// WHEN
	Storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(Data));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_complete_update_added_THEN_complete_update_merged)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const int kEventValue = 7332;
	const double kUpdateValue = 7333;

	auto Update = CreateTestComponentUpdate(kTestComponentId, kTestValue);
	AddTestEvent(&Update, kEventValue);
	auto CompleteUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push({ kTestEntityId, CompleteUpdate.DeepCopy(), Update.DeepCopy() });
	TArray<EntityComponentUpdate> ExpectedEvents;
	ExpectedEvents.Push(EntityComponentUpdate{ kTestEntityId, Update.DeepCopy() });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(Update));

	// WHEN
	Storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(CompleteUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_a_complete_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const int kEventValue = 7332;
	const double kUpdateValue = 7333;

	auto CompleteUpdate = CreateTestComponentData(kTestComponentId, kTestValue);
	auto Update = CreateTestComponentUpdate(kTestComponentId, kUpdateValue);
	AddTestEvent(&Update, kEventValue);
	auto additionalEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);

	auto ExpectedCompleteUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
	auto expectedEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
	AddTestEvent(&expectedEvent, kEventValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates{};
	TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates;
	ExpectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ kTestEntityId, MoveTemp(ExpectedCompleteUpdate), MoveTemp(expectedEvent) });

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(CompleteUpdate));

	// WHEN
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(Update));
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(additionalEvent));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_multiple_updates_WHEN_component_removed_THEN_its_updates_removed)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kComponentIdToRemove = 1347;
	const Worker_ComponentId kComponentIdToKeep = 1348;
	const double kTestValue = 7331;
	const int kEventValue = 7332;
	const double kUpdateValue = 7333;

	auto CompleteUpdateToRemove = CreateTestComponentData(kComponentIdToRemove, kTestValue);
	auto EventToRemove = CreateTestComponentEvent(kComponentIdToRemove, kEventValue);

	auto UpdateToKeep = CreateTestComponentUpdate(kComponentIdToKeep, kUpdateValue);

	TArray<EntityComponentUpdate> ExpectedUpdates;
	ExpectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, UpdateToKeep.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> ExpectedEvents = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(CompleteUpdateToRemove));
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(EventToRemove));
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(UpdateToKeep));

	// WHEN
	Storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_component_removed_THEN_its_update_removed)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kComponentIdToRemove = 1347;
	const double kUpdateValue = 7333;

	auto Update = CreateTestComponentUpdate(kComponentIdToRemove, kUpdateValue);

	const TArray<EntityComponentUpdate> ExpectedUpdates = {};
	const TArray<EntityComponentCompleteUpdate> ExpectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> ExpectedEvents = {};

	EntityComponentUpdateRecord Storage;
	Storage.AddComponentUpdate(kTestEntityId, MoveTemp(Update));

	// WHEN
	Storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(Storage.GetUpdates(), ExpectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(Storage.GetCompleteUpdates(), ExpectedCompleteUpdates));

	return true;
}
