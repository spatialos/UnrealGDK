// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTRECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentRecord, TestName)

using namespace SpatialGDK;

ENTITYCOMPONENTRECORD_TEST(CanAddComponent)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto testData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentId> expectedComponentsRemoved = {};
	TArray<EntityComponentData> expectedComponentsAdded;
	expectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, testData.DeepCopy() });

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, MoveTemp(testData));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanRemoveComponent)
{
	const EntityComponentId kEntityComponentId = { 1337, 1338 };

	EntityComponentRecord storage;
	storage.RemoveComponent(kEntityComponentId.EntityId, kEntityComponentId.ComponentId);

	const TArray<EntityComponentData> expectedComponentsAdded = {};
	const TArray<EntityComponentId> expectedComponentsRemoved = { kEntityComponentId };

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));

	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanAddThenRemoveComponent)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto testData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentData> expectedComponentsAdded = {};
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, MoveTemp(testData));
	storage.RemoveComponent(kTestEntityId, kTestComponentId);

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

// This should not produce the component added - just cancel removing it
ENTITYCOMPONENTRECORD_TEST(CanRemoveThenAddComponent)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto testData = CreateTestComponentData(kTestComponentId, kTestValue);

	const TArray<EntityComponentData> expectedComponentsAdded = {};
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.RemoveComponent(kTestEntityId, kTestComponentId);
	storage.AddComponent(kTestEntityId, MoveTemp(testData));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanApplyUpdateToComponentAdded)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const double kTestUpdateValue = 7332;

	ComponentData testData = CreateTestComponentData(kTestComponentId, kTestValue);
	ComponentUpdate testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);
	ComponentData expectedData = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentData> expectedComponentsAdded;
	expectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, MoveTemp(expectedData) });
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, MoveTemp(testData));
	storage.AddUpdate(kTestEntityId, MoveTemp(testUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanNotApplyUpdateIfNoComponentAdded)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestUpdateValue = 7332;

	ComponentUpdate testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);

	const TArray<EntityComponentData> expectedComponentsAdded = {};
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddUpdate(kTestEntityId, MoveTemp(testUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanApplyCompleteUpdateToComponentAdded)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;
	const double kTestUpdateValue = 7332;

	ComponentData testData = CreateTestComponentData(kTestComponentId, kTestValue);
	ComponentData testUpdate = CreateTestComponentData(kTestComponentId, kTestUpdateValue);
	ComponentData expectedData = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	TArray<EntityComponentData> expectedComponentsAdded;
	expectedComponentsAdded.Push(EntityComponentData{ kTestEntityId, MoveTemp(expectedData) });
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, MoveTemp(testData));
	storage.AddComponentAsUpdate(kTestEntityId, MoveTemp(testUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanNotApplyCompleteUpdateIfNoComponentAdded)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestUpdateValue = 7332;

	ComponentData testUpdate = CreateTestComponentData(kTestComponentId, kTestUpdateValue);

	const TArray<EntityComponentData> expectedComponentsAdded = {};
	const TArray<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponentAsUpdate(kTestEntityId, MoveTemp(testUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

