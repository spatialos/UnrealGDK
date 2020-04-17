// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

using namespace SpatialGDK;

namespace
{
	// TODO(Alex): templatize?
	bool AreEquivalent(const TArray<EntityComponentUpdate>& lhs, const TArray<EntityComponentUpdate>& rhs)
	{
		return AreEquivalent(lhs, rhs, CompareEntityComponentUpdates);
	}

	bool AreEquivalent(const TArray<EntityComponentCompleteUpdate>& lhs, const TArray<EntityComponentCompleteUpdate>& rhs)
	{
		return AreEquivalent(lhs, rhs, CompareEntityComponentCompleteUpdates);
	}

}  // anonymous namespace

ENTITYCOMPONENTUPDATERECORD_TEST(CanAddUpdate)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);

	TArray<EntityComponentUpdate> expectedUpdates;
	expectedUpdates.Push(EntityComponentUpdate{ kTestEntityId, testUpdate.DeepCopy() });
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(testUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanMergeUpdate)
{
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
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(secondUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanAddCompleteUpdate)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;
	const double kTestValue = 7331;

	auto data = CreateTestComponentData(kTestComponentId, kTestValue);

	TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates;
	expectedCompleteUpdates.Push(EntityComponentCompleteUpdate{ kTestEntityId, data.DeepCopy(), ComponentUpdate(kTestComponentId) });
	const TArray<EntityComponentUpdate> expectedUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(data));

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanMergeCompleteUpdate)
{
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
	storage.AddComponentDataAsUpdate(kTestEntityId, MoveTemp(completeUpdate));

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanMergeOntoACompleteUpdate)
{
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
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(update));
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(additionalEvent));

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanRemoveComponentWithCompleteUpdate)
{
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

	storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanRemoveComponent_Update)
{
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kComponentIdToRemove = 1347;
	const double kUpdateValue = 7333;

	auto update = CreateTestComponentUpdate(kComponentIdToRemove, kUpdateValue);

	const TArray<EntityComponentUpdate> expectedUpdates = {};
	const TArray<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
	const TArray<EntityComponentUpdate> expectedEvents = {};

	EntityComponentUpdateRecord storage;
	storage.AddComponentUpdate(kTestEntityId, MoveTemp(update));

	storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);

	TestTrue(TEXT(""), AreEquivalent(storage.GetUpdates(), expectedUpdates));
	TestTrue(TEXT(""), AreEquivalent(storage.GetCompleteUpdates(), expectedCompleteUpdates));

	return true;
}
