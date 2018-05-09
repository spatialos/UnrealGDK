// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Tests/SpatialOSTest.h"

#include "SimpleEntitySpawnerBlockTest.generated.h"

UCLASS()
class SDK_API USimpleEntitySpawnerBlockTest : public USpatialOSTest
{
  GENERATED_BODY()
public:
  UFUNCTION()
  void TestCreateEntity() const;

  UFUNCTION()
  void TestInitialComponentValuesAreAppliedBeforeIncomingUpdatesDuringEntityCreation() const;
};
