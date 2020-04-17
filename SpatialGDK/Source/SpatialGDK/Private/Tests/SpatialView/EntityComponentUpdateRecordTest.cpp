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

	auto testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);

	TArray<EntityComponentUpdate> expectedUpdates;
	expectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, testUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;

	// WHEN
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(testUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_new_update_added_THEN_new_update_merged)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const double kTestUpdateValue = 7332;

	auto firstUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);
	auto secondUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentUpdate> expectedUpdates;
	expectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, secondUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(firstUpdate));

	// WHEN
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(secondUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_empty_update_record_WHEN_complete_update_added_THEN_update_record_has_complete_update)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto data = CreateTestComponentData(kTestComponentId, kTestValue);

	TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates;
	expectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ kTestEntityId, data.DeepCopy(), ComponentUpdate(kTestComponentId) });
	const TArray<EntityComponentUpdate> expectedUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;

	// WHEN
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(data));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

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

	auto update = CreateTestComponentUpdate(kTestComponentId, kTestValue);
	AddTestEvent(&update, kEventValue);
	auto completeUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);

	const TArray<EntityComponentUpdate> expectedUpdates = {};
	TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates;
	expectedCompleteUpdates.Push({ kTestEntityId, completeUpdate.DeepCopy(), update.DeepCopy() });
	TArray<EntityComponentUpdate> expectedEvents;
	expectedEvents.Push(EntityComponentUpdate{ kTestEntityId, update.DeepCopy() });

	EntityComponentUpdateRecord storage;
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(update));

	// WHEN
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(completeUpdate));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

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

	auto completeUpdate = CreateTestComponentData(kTestComponentId, kTestValue);
	auto update = CreateTestComponentUpdate(kTestComponentId, kUpdateValue);
	AddTestEvent(&update, kEventValue);
	auto additionalEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);

	auto expectedCompleteUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
	auto expectedEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
	AddTestEvent(&expectedEvent, kEventValue);

	const TArray<EntityComponentUpdate> expectedUpdates{};
	TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates;
	expectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ kTestEntityId, MoveTemp(expectedCompleteUpdate), MoveTemp(expectedEvent) });

	EntityComponentUpdateRecord storage;
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(completeUpdate));

	// WHEN
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(update));
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(additionalEvent));

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

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

	auto completeUpdateToRemove = CreateTestComponentData(kComponentIdToRemove, kTestValue);
	auto eventToRemove = CreateTestComponentEvent(kComponentIdToRemove, kEventValue);

	auto updateToKeep = CreateTestComponentUpdate(kComponentIdToKeep, kUpdateValue);

	TArray<EntityComponentUpdate> expectedUpdates;
	expectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, updateToKeep.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(completeUpdateToRemove));
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(eventToRemove));
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(updateToKeep));

	// WHEN
	storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(GIVEN_update_record_with_update_WHEN_component_removed_THEN_its_update_removed)
{
	// GIVEN
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kComponentIdToRemove = 1347;
	const double kUpdateValue = 7333;

	auto update = CreateTestComponentUpdate(kComponentIdToRemove, kUpdateValue);

	const TArray<EntityComponentUpdate> expectedUpdates = {};
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(update));

	// WHEN
	storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	// THEN
	TestTrue(TEXT("Updates are equal to expected"), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT("Complete updates are equal to expected"), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}
