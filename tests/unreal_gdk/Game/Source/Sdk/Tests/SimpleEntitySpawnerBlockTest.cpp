// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimpleEntitySpawnerBlockTest.h"

#include "EntityPipeline.h"
#include "EntityRegistry.h"

#include "SimpleEntitySpawnerBlock.h"
#include "SpatialOSMockViewTypes.h"
#include "SpatialOSMockWorkerTypes.h"
#include "TestData2Component.h"
#include "Utils/MockSpatialOS.h"

#include "Runtime/Core/Public/Templates/SharedPointer.h"

#define ENTITY_BLUEPRINTS_FOLDER "/Game/EntityBlueprints"

DEFINE_SPATIAL_AUTOMATION_TEST(SimpleEntitySpawnerBlock)

USTRUCT()
struct FSimpleEntitySpawnerBlockTestFixture
{
  TSharedPtr<MockConnection> Connection;

  TSharedPtr<MockView> View;

  UPROPERTY()
  UEntityRegistry* EntityRegistry;

  UPROPERTY()
  UEntityPipeline* EntityPipeline;

  UPROPERTY()
  USimpleEntitySpawnerBlock* EntitySpawnerBlock;

  UPROPERTY()
  UCallbackDispatcher* CallbackDispatcher;

  FSimpleEntitySpawnerBlockTestFixture()
  {
    Connection = MockSpatialOS::CreateMockConnection();
    View = MockSpatialOS::CreateMockView();

    CallbackDispatcher = NewObject<UCallbackDispatcher>(UCallbackDispatcher::StaticClass());
    CallbackDispatcher->Init(View);

    EntityRegistry = NewObject<UEntityRegistry>(UEntityRegistry::StaticClass());
    TArray<FString> BlueprintPaths;
    BlueprintPaths.Add(TEXT(ENTITY_BLUEPRINTS_FOLDER));
    EntityRegistry->RegisterEntityBlueprints(BlueprintPaths);

    EntitySpawnerBlock = NewObject<USimpleEntitySpawnerBlock>();
    EntitySpawnerBlock->Init(EntityRegistry);

    EntityPipeline = NewObject<UEntityPipeline>(UEntityPipeline::StaticClass());
    EntityPipeline->AddBlock(EntitySpawnerBlock);
    EntityPipeline->Init(View, CallbackDispatcher);
  }
};

void USimpleEntitySpawnerBlockTest::TestCreateEntity() const
{
  FSimpleEntitySpawnerBlockTestFixture Fixture;

  // mock ops
  const worker::EntityId MockEntityId = 10;
  Fixture.Connection->PushAddEntityOpToOpsList(MockEntityId);
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::TestData2>(MockEntityId,
                                                                         improbable::TestType2(10));
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::Position>(
      MockEntityId, improbable::PositionData(improbable::Coordinates(1, 1, 1)));
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::Metadata>(
      MockEntityId, improbable::MetadataData("ExampleActor"));
  // end mock ops

  Fixture.View->Process(Fixture.Connection->GetOpList(0));
  // Create a world without informing the engine about its existence.
  UWorld* DummyWorld = UWorld::CreateWorld(EWorldType::Game, false);
  Fixture.EntityPipeline->ProcessOps(TWeakPtr<SpatialOSView>(Fixture.View),
                                     TWeakPtr<MockConnection>(Fixture.Connection), DummyWorld);

  auto TestEntity = Fixture.EntityRegistry->GetActorFromEntityId(MockEntityId);
  T->TestTrue(TEXT("entity should be created"), TestEntity != nullptr);
}

void USimpleEntitySpawnerBlockTest::
    TestInitialComponentValuesAreAppliedBeforeIncomingUpdatesDuringEntityCreation() const
{
  FSimpleEntitySpawnerBlockTestFixture Fixture;

  // mock ops
  const worker::EntityId MockEntityId = 10;
  Fixture.Connection->PushAddEntityOpToOpsList(MockEntityId);
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::Position>(
      MockEntityId, improbable::PositionData(improbable::Coordinates(1, 1, 1)));
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::Metadata>(
      MockEntityId, improbable::MetadataData("ExampleActor"));
  // end mock ops

  Fixture.View->Process(Fixture.Connection->GetOpList(0));
  // Create a world without informing the engine about its existence.
  UWorld* DummyWorld = UWorld::CreateWorld(EWorldType::Game, false);
  Fixture.EntityPipeline->ProcessOps(TWeakPtr<SpatialOSView>(Fixture.View),
                                     TWeakPtr<MockConnection>(Fixture.Connection), DummyWorld);

  // DEV-1657: We have to add these ops in a separate ops list due to a bug where the components
  // might not get initialized properly if component updates
  // happen on the same frame.
  Fixture.Connection->PushAddComponentOpToOpsList<improbable::TestData2>(MockEntityId,
                                                                         improbable::TestType2(1));

  auto UpdateValue = improbable::TestData2::Update();
  UpdateValue.set_int32_property(0);
  Fixture.Connection->PushUpdateComponentOpToOpsList<improbable::TestData2>(MockEntityId,
                                                                            UpdateValue);

  Fixture.View->Process(Fixture.Connection->GetOpList(0));
  Fixture.EntityPipeline->ProcessOps(TWeakPtr<SpatialOSView>(Fixture.View),
                                     TWeakPtr<MockConnection>(Fixture.Connection), DummyWorld);

  auto TestEntity = Fixture.EntityRegistry->GetActorFromEntityId(MockEntityId);
  auto* TestData2Component = TestEntity->FindComponentByClass<UTestData2Component>();

  T->TestTrue(TEXT("Components updates are not overwritten by the initial update"),
              TestData2Component->Int32Property == 0);
}
