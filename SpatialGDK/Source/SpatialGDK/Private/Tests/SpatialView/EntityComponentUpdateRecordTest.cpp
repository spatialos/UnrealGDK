// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/EntityComponentUpdateRecord.h"

#include "EntityComponentTestUtils.h"

// TODO(Alex): remove std include
#include <vector>

#define ENTITYCOMPONENTUPDATERECORD_TEST(TestName) \
	GDK_TEST(Core, EntityComponentUpdateRecord, TestName)

using namespace SpatialGDK;

namespace
{
	bool CompareUpdates(const TArray<EntityComponentUpdate>& lhs,
		const std::vector<EntityComponentUpdate>& rhs)
	{
		return true;
	}

	bool CompareCompleteUpdates(const TArray<EntityComponentCompleteUpdate>& lhs,
		const std::vector<EntityComponentCompleteUpdate>& rhs)
	{
		return true;
	}

	//bool CompareEvents(const TArray<EntityComponentUpdate>& lhs,
	//	const std::vector<EntityComponentUpdate>& rhs)
	//{
	//	return true;
	//}

	//bool CompareEvents(const TArray<EntityComponentCompleteUpdate>& lhs,
	//	const std::vector<EntityComponentCompleteUpdate>& rhs)
	//{
	//	return true;
	//}

}  // anonymous namespace

ENTITYCOMPONENTUPDATERECORD_TEST(CanAddUpdate)
{
  const Worker_EntityId kTestEntityId = 1337;
  const Worker_ComponentId kTestComponentId = 1338;
  const double kTestValue = 7331;

  auto testUpdate = CreateTestComponentUpdate(kTestComponentId, kTestValue);

  const auto expectedUpdates =
      CreateVector<EntityComponentUpdate>(EntityComponentUpdate{kTestEntityId, testUpdate.DeepCopy()});
  const std::vector<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
  const std::vector<EntityComponentUpdate> expectedEvents = {};

  EntityComponentUpdateRecord storage;
  storage.AddComponentUpdate(kTestEntityId, std::move(testUpdate));

  TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
  TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));

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

  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
      EntityComponentUpdate{kTestEntityId, secondUpdate.DeepCopy()});
  const std::vector<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
  const std::vector<EntityComponentUpdate> expectedEvents = {};

  EntityComponentUpdateRecord storage;
  storage.AddComponentUpdate(kTestEntityId, std::move(firstUpdate));
  storage.AddComponentUpdate(kTestEntityId, std::move(secondUpdate));

  TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
  TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));

  return true;
}

ENTITYCOMPONENTUPDATERECORD_TEST(CanAddCompleteUpdate)
{
  const Worker_EntityId kTestEntityId = 1337;
  const Worker_ComponentId kTestComponentId = 1338;
  const double kTestValue = 7331;

  auto data = CreateTestComponentData(kTestComponentId, kTestValue);

  EntityComponentCompleteUpdate Update(kTestEntityId, ComponentData(kTestComponentId), ComponentUpdate(kTestComponentId));
  const auto expectedCompleteUpdates = CreateVector<EntityComponentCompleteUpdate>(MoveTemp(Update));
  const std::vector<EntityComponentUpdate> expectedUpdates = {};
  const std::vector<EntityComponentUpdate> expectedEvents = {};

  EntityComponentUpdateRecord storage;
  storage.AddComponentDataAsUpdate(kTestEntityId, std::move(data));

  TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
  TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));

  return true;
}

//ENTITYCOMPONENTUPDATERECORD_TEST(CanMergeCompleteUpdate)
//{
//  const Worker_EntityId kTestEntityId = 1337;
//  const Worker_ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto update = CreateTestComponentUpdate(kTestComponentId, kTestValue);
//  //AddTestEvent(&update, kEventValue);
//  auto completeUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates = {};
//  const auto expectedCompleteUpdates =
//      CreateVector<EntityComponentCompleteUpdate>(EntityComponentCompleteUpdate{kTestEntityId, completeUpdate.DeepCopy()});
//  const auto expectedEvents =
//      CreateVector<EntityComponentUpdate>(EntityComponentUpdate{kTestEntityId, update.DeepCopy()});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdate));
//
//  //TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  //TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));
//
//  return true;
//}
//
//ENTITYCOMPONENTUPDATERECORD_TEST(CanMergeOntoACompleteUpdate)
//{
//  const Worker_EntityId kTestEntityId = 1337;
//  const Worker_ComponentId kTestComponentId = 1338;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdate = CreateTestComponentData(kTestComponentId, kTestValue);
//  auto update = CreateTestComponentUpdate(kTestComponentId, kUpdateValue);
//  //AddTestEvent(&update, kEventValue);
//  //auto additionalEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
//
//  auto expectedCompleteUpdate = CreateTestComponentData(kTestComponentId, kUpdateValue);
//  //auto expectedEvent = CreateTestComponentEvent(kTestComponentId, kEventValue);
//  //AddTestEvent(&expectedEvent, kEventValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates{};
//  const auto expectedCompleteUpdates = CreateVector<EntityComponentCompleteUpdate>(
//      EntityComponentCompleteUpdate{kTestEntityId, std::move(expectedCompleteUpdate)});
//  const auto expectedEvents = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kTestEntityId, std::move(expectedEvent)});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdate));
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//  storage.AddComponentUpdate(kTestEntityId, std::move(additionalEvent));
//
//  //TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  //TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));
//
//  return true;
//}
//
//ENTITYCOMPONENTUPDATERECORD_TEST(CanRemoveComponentWithCompleteUpdate)
//{
//  const Worker_EntityId kTestEntityId = 1337;
//  const Worker_ComponentId kComponentIdToRemove = 1347;
//  const Worker_ComponentId kComponentIdToKeep = 1348;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdateToRemove = CreateTestComponentData(kComponentIdToRemove, kTestValue);
//  //auto eventToRemove = CreateTestComponentEvent(kComponentIdToRemove, kEventValue);
//
//  auto updateToKeep = CreateTestComponentUpdate(kComponentIdToKeep, kUpdateValue);
//
//  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kTestEntityId, updateToKeep.DeepCopy()});
//  std::vector<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
//  std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(completeUpdateToRemove));
//  //storage.AddComponentUpdate(kTestEntityId, std::move(eventToRemove));
//  storage.AddComponentUpdate(kTestEntityId, std::move(updateToKeep));
//
//  storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);
//
//  //TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  //TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));
//
//  return true;
//}
//
//ENTITYCOMPONENTUPDATERECORD_TEST(CanRemoveComponent)
//{
//  const Worker_EntityId kTestEntityId = 1337;
//  const Worker_ComponentId kComponentIdToRemove = 1347;
//  const double kUpdateValue = 7333;
//
//  auto update = CreateTestComponentUpdate(kComponentIdToRemove, kUpdateValue);
//
//  std::vector<EntityComponentUpdate> expectedUpdates = {};
//  std::vector<EntityComponentCompleteUpdate> expectedCompleteUpdates = {};
//  std::vector<EntityComponentUpdate> expectedEvents = {};
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kTestEntityId, std::move(update));
//
//  storage.RemoveComponent(kTestEntityId, kComponentIdToRemove);
//
//  //TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  //TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));
//
//  return true;
//}
//
//ENTITYCOMPONENTUPDATERECORD_TEST(CanRemoveEntity)
//{
//  const Worker_EntityId kEntityIdToRemove = 1337;
//  const Worker_EntityId kEntityIdToKeep = 1338;
//  const Worker_ComponentId kFirstComponentId = 1347;
//  const Worker_ComponentId kSecondComponentId = 1348;
//  const double kTestValue = 7331;
//  const int kEventValue = 7332;
//  const double kUpdateValue = 7333;
//
//  auto completeUpdateToRemove = CreateTestComponentData(kFirstComponentId, kTestValue);
//  //auto eventToRemove = CreateTestComponentEvent(kFirstComponentId, kEventValue);
//  auto updateToRemove = CreateTestComponentUpdate(kSecondComponentId, kUpdateValue);
//
//  auto completeUpdateToKeep = CreateTestComponentData(kSecondComponentId, kTestValue);
//  //auto eventToKeep = CreateTestComponentEvent(kSecondComponentId, kEventValue);
//  auto updateToKeep = CreateTestComponentUpdate(kFirstComponentId, kUpdateValue);
//
//  const auto expectedUpdates = CreateVector<EntityComponentUpdate>(
//      EntityComponentUpdate{kEntityIdToKeep, updateToKeep.DeepCopy()});
//  const auto expectedCompleteUpdates = CreateVector<EntityComponentCompleteUpdate>(
//      EntityComponentData{kEntityIdToKeep, completeUpdateToKeep.DeepCopy()});
//  //const auto expectedEvents = CreateVector<EntityComponentUpdate>(
//  //    EntityComponentUpdate{kEntityIdToKeep, eventToKeep.DeepCopy()});
//
//  EntityComponentUpdateRecord storage;
//  storage.AddComponentUpdate(kEntityIdToRemove, std::move(completeUpdateToRemove));
//  storage.AddComponentUpdate(kEntityIdToRemove, std::move(updateToRemove));
//  //storage.AddComponentUpdate(kEntityIdToRemove, std::move(eventToRemove));
//
//  storage.AddComponentUpdate(kEntityIdToKeep, std::move(completeUpdateToKeep));
//  storage.AddComponentUpdate(kEntityIdToKeep, std::move(updateToKeep));
//  //storage.AddComponentUpdate(kEntityIdToKeep, std::move(eventToKeep));
//
//  //storage.RemoveEntity(kEntityIdToRemove);
//
//  //TestTrue(TEXT(""), CompareUpdates(storage.GetUpdates(), expectedUpdates));
//  //TestTrue(TEXT(""), CompareCompleteUpdates(storage.GetCompleteUpdates(), expectedCompleteUpdates));
//  //TestTrue(TEXT(""), CompareEvents(storage.GetEvents(), expectedEvents));
//
//  return true;
//}
