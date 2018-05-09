// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "ExampleActor.h"

#include "PositionComponent.h"
#include "TestData2Component.h"

AExampleActor::AExampleActor()
{
  PrimaryActorTick.bCanEverTick = true;
  PositionComponent = CreateDefaultSubobject<UPositionComponent>(TEXT("PositionComponent"));
  TestData2Component = CreateDefaultSubobject<UTestData2Component>(TEXT("TestData2Component"));
}

void AExampleActor::BeginPlay()
{
  Super::BeginPlay();
}

void AExampleActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}
