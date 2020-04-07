// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentRecord.h"

#include "EntityComponentTestUtils.h"

// TODO(Alex): remove std include
#include <vector>

#define ENTITYCOMPONENTRECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentRecord, TestName)

using namespace SpatialGDK;

namespace
{
	bool AreEquivalent(const TArray<EntityComponentData>& lhs,
		const std::vector<EntityComponentData>& rhs)
	{
		if (lhs.Num() == rhs.size())
		{
			for (int i = 0; i < lhs.Num(); i++)
			{
				if (!CompareEntityComponentData(lhs[i], rhs[i]))
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool AreEquivalent(const TArray<EntityComponentId>& lhs,
		const std::vector<EntityComponentId>& rhs)
	{
		if (lhs.Num() == rhs.size())
		{
			for (int i = 0; i < lhs.Num(); i++)
			{
				if (!(lhs[i] == rhs[i]))
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}
}  // anonymous namespace

ENTITYCOMPONENTRECORD_TEST(CanAddComponent)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto testData = CreateTestComponentData(kTestComponentId, kTestValue);

	const std::vector<EntityComponentId> expectedComponentsRemoved = {};
	const auto expectedComponentsAdded = CreateVector<EntityComponentData>(EntityComponentData{ kTestEntityId, testData.DeepCopy() });

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, std::move(testData));

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanRemoveComponent)
{
	const EntityComponentId kEntityComponentId = { 1337, 1338 };

	EntityComponentRecord storage;
	storage.RemoveComponent(kEntityComponentId.EntityId, kEntityComponentId.ComponentId);

	const std::vector<EntityComponentData> expectedComponentsAdded = {};
	const std::vector<EntityComponentId> expectedComponentsRemoved = { kEntityComponentId };

	//TestTrue(TEXT(""), removed);
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

	const std::vector<EntityComponentData> expectedComponentsAdded = {};
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, std::move(testData));
	storage.RemoveComponent(kTestEntityId, kTestComponentId);

	//TestTrue(TEXT(""), added);
	//TestFalse(TEXT(""), removed);
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

	const std::vector<EntityComponentData> expectedComponentsAdded = {};
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.RemoveComponent(kTestEntityId, kTestComponentId);
	storage.AddComponent(kTestEntityId, std::move(testData));

	//TestTrue(TEXT(""), removed);
	//TestFalse(TEXT(""), added);
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

	const auto expectedComponentsAdded = CreateVector<EntityComponentData>(
		EntityComponentData{ kTestEntityId, std::move(expectedData) });
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, std::move(testData));
	storage.AddUpdate(kTestEntityId, MoveTemp(testUpdate));

	//TestTrue(TEXT(""), added);
	//TestTrue(TEXT(""), updated);
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

	const std::vector<EntityComponentData> expectedComponentsAdded = {};
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddUpdate(kTestEntityId, MoveTemp(testUpdate));

	//TestFalse(TEXT(""), updated);
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

	const auto expectedComponentsAdded = CreateVector<EntityComponentData>(
		EntityComponentData{ kTestEntityId, std::move(expectedData) });
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kTestEntityId, std::move(testData));
	storage.AddComponentAsUpdate(kTestEntityId, std::move(testUpdate));

	//TestTrue(TEXT(""), added);
	//TestTrue(TEXT(""), updated);
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

	const std::vector<EntityComponentData> expectedComponentsAdded = {};
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	 storage.AddComponentAsUpdate(kTestEntityId, std::move(testUpdate));

	//TestFalse(TEXT(""), updated);
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}

ENTITYCOMPONENTRECORD_TEST(CanRemoveEntity)
{
	const Worker_EntityId kEntityIdToRemove = 1337;
	const Worker_EntityId kEntityIdToKeep = 1338;
	const Worker_ComponentId kFirstComponentId = 1339;
	const Worker_ComponentId kSecondComponentId = 1339;
	const double kTestValue = 7331;

	ComponentData firstDataToRemove = CreateTestComponentData(kFirstComponentId, kTestValue);
	ComponentData secondDataToRemove = CreateTestComponentData(kSecondComponentId, kTestValue);
	ComponentData dataToKeep = CreateTestComponentData(kFirstComponentId, kTestValue);

	const auto expectedComponentsAdded =
		CreateVector<EntityComponentData>(EntityComponentData{ kEntityIdToKeep, dataToKeep.DeepCopy() });
	const std::vector<EntityComponentId> expectedComponentsRemoved = {};

	EntityComponentRecord storage;
	storage.AddComponent(kEntityIdToRemove, std::move(firstDataToRemove));
	storage.AddComponent(kEntityIdToRemove, std::move(secondDataToRemove));
	storage.AddComponent(kEntityIdToKeep, std::move(dataToKeep));
	storage.RemoveComponent(kEntityIdToRemove, kFirstComponentId);
	storage.RemoveComponent(kEntityIdToRemove, kSecondComponentId);

	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsAdded(), expectedComponentsAdded));
	TestTrue(TEXT(""), AreEquivalent(storage.GetComponentsRemoved(), expectedComponentsRemoved));
	return true;
}
