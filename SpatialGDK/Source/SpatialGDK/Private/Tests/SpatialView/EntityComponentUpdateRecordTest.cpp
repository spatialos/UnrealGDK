// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

// TODO(Alex): remove std include
#include <vector>

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

using namespace SpatialGDK;

namespace {

	bool CompareUpdates(const std::vector<EntityComponentUpdate>& lhs,
		const std::vector<EntityComponentUpdate>& rhs) {
		return std::is_permutation(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
			CompareEntityComponentUpdates);
	}

	bool CompareCompleteUpdates(const std::vector<EntityComponentData>& lhs,
		const std::vector<EntityComponentData>& rhs) {
		return std::is_permutation(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
			CompareEntityComponentData);
	}

	bool CompareEvents(const std::vector<EntityComponentUpdate>& lhs,
		const std::vector<EntityComponentUpdate>& rhs) {
		return std::is_permutation(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
			CompareEntityComponentUpdateEvents);
	}

}  // anonymous namespace

//TEST(UpdateRecordTest, CanAddUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//
//  auto testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);
//
//  const auto expectedUpdates =
//      CreateVector<EntityComponentUpdate>(EntityComponentUpdate{kTestEntityId, testUpdate.DeepCopy()});
//  const std::vector<EntityComponentData> expectedCompleteUpdates = {};
//  const std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(testUpdate));
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanMergeUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//  const double kTestUpdateValue = 7332;
//
//  auto firstUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);
//  auto secondUpdate = CreateTestComponentUpdate(kTestComponentId, kTestUpdateValue);
//
//  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kTestEntityId, secondUpdate.DeepCopy()});
//  const std::vector<EntityComponentData> expectedCompleteUpdates = {};
//  const std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(firstUpdate));
//  storage.AddComponentUpdate(kTestEntityId, std::move(secondUpdate));
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanAddCompleteUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//
//  auto data = CreateTestComponentData(kTestComponentId, kTestValue);
//
//  const auto expectedCompleteUpdates =
//      CreateVector<EntityComponentData>(EntityComponentData{kTestEntityId, data.DeepCopy()});
//  const std::vector<EntityComponentUpdate> expectedUpdates = {};
//  const std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(data));
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanMergeCompleteUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto update = CreateTestComponentUpdate(kTestComponentId, kTestValue);
//  AddTestEvent(&update, kEventValue);
//  auto completeUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates = {};
//  const auto expectedCompleteUpdates =
//      CreateVector<EntityComponentData>(EntityComponentData{kTestEntityId, completeUpdate.DeepCopy()});
//  const auto expectedEvents =
//      CreateVector<EntityComponentUpdate>(EntityComponentUpdate{kTestEntityId, update.DeepCopy()});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdate));
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanMergeOntoACompleteUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdate = CreateTestComponentData(kTestComponentId, kTestValue);
//  auto update = CreateTestComponentUpdate(kTestComponentId, kUpdateValue);
//  AddTestEvent(&update, kEventValue);
//  auto additionalEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
//
//  auto expectedCompleteUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
//  auto expectedEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
//  AddTestEvent(&expectedEvent, kEventValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates{};
//  const auto expectedCompleteUpdates = CreateVector<EntityComponentData>(
//      EntityComponentData{kTestEntityId, std::move(expectedCompleteUpdate)});
//  const auto expectedEvents = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kTestEntityId, std::move(expectedEvent)});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdate));
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//  storage.AddComponentUpdate(kTestEntityId, std::move(additionalEvent));
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanRemoveComponentWithCompleteUpdate) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kComponentIdToRemove = 1347;
//  const ComponentId kComponentIdToKeep = 1348;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdateToRemove = CreateTestComponentData(kComponentIdToRemove, kTestValue);
//  auto eventToRemove = CreateTestComponentEvent(kComponentIdToRemove, kEventValue);
//
//  auto updateToKeep = CreateTestComponentUpdate(kComponentIdToKeep, kUpdateValue);
//
//  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kTestEntityId, updateToKeep.DeepCopy()});
//  std::vector<EntityComponentData> expectedCompleteUpdates = {};
//  std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdateToRemove));
//  storage.AddComponentUpdate(kTestEntityId, std::move(eventToRemove));
//  storage.AddComponentUpdate(kTestEntityId, std::move(updateToKeep));
//
//  storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanRemoveComponent) {
//  const EntityId kTestEntityId = 1337;
//  const ComponentId kComponentIdToRemove = 1347;
//  const double kUpdateValue = 7333;
//
//  auto update = CreateTestComponentUpdate(kComponentIdToRemove, kUpdateValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates = {};
//  std::vector<EntityComponentData> expectedCompleteUpdates = {};
//  std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//
//  storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
//
//TEST(UpdateRecordTest, CanRemoveEntity) {
//  const EntityId kEntityIdToRemove = 1337;
//  const EntityId kEntityIdToKeep = 1338;
//  const ComponentId kFirstComponentId = 1347;
//  const ComponentId kSecondComponentId = 1348;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdateToRemove = CreateTestComponentData(kFirstComponentId, kTestValue);
//  auto eventToRemove = CreateTestComponentEvent(kFirstComponentId, kEventValue);
//  auto updateToRemove = CreateTestComponentUpdate(kSecondComponentId, kUpdateValue);
//
//  auto completeUpdateToKeep = CreateTestComponentData(kSecondComponentId, kTestValue);
//  auto eventToKeep = CreateTestComponentEvent(kSecondComponentId, kEventValue);
//  auto updateToKeep = CreateTestComponentUpdate(kFirstComponentId, kUpdateValue);
//
//  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kEntityIdToKeep, updateToKeep.DeepCopy()});
//  const auto expectedCompleteUpdates = CreateVector<EntityComponentData>(
//      EntityComponentData{kEntityIdToKeep, completeUpdateToKeep.DeepCopy()});
//  const auto expectedEvents = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kEntityIdToKeep, eventToKeep.DeepCopy()});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kEntityIdToRemove, std::move(completeUpdateToRemove));
//  storage.AddComponentUpdate(kEntityIdToRemove, std::move(updateToRemove));
//  storage.AddComponentUpdate(kEntityIdToRemove, std::move(eventToRemove));
//
//  storage.AddComponentUpdate(kEntityIdToKeep, std::move(completeUpdateToKeep));
//  storage.AddComponentUpdate(kEntityIdToKeep, std::move(updateToKeep));
//  storage.AddComponentUpdate(kEntityIdToKeep, std::move(eventToKeep));
//
//  storage.RemoveEntity(kEntityIdToRemove);
//
//  ASSERT_TRUE(CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  ASSERT_TRUE(CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  ASSERT_TRUE(CompareEvents(storage.GetEvents(), expectedEvents));
//}
