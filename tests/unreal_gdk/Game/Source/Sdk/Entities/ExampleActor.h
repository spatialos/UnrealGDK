// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PositionComponent.h"
#include "TestData2Component.h"
#include "UObject/ObjectMacros.h"

#include "ExampleActor.generated.h"

UCLASS()
class AExampleActor : public AActor
{
  GENERATED_BODY()

public:
  AExampleActor();

  UPROPERTY()
  UPositionComponent* PositionComponent;

  UPROPERTY()
  UTestData2Component* TestData2Component;

protected:
  void BeginPlay() override;
  void EndPlay(EEndPlayReason::Type EndPlayReason) override;
};
