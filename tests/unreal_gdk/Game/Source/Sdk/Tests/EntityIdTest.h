// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EntityId.h"
#include "Tests/SpatialOSTest.h"

#include "EntityIdTest.generated.h"

UCLASS()
class SDK_API UEntityIdTest : public USpatialOSTest
{
  GENERATED_BODY()

  UFUNCTION()
  void TestDefaultInitialize() const
  {
    FEntityId Invalid;
    T->TestTrue(TEXT("Default initializes to invalid EntityId"),
                Invalid.ToSpatialEntityId() == worker::EntityId());
  }

  UFUNCTION()
  void TestFEntityIdEquality() const
  {
    T->TestTrue(TEXT("Equality with FEntityId"),
                FEntityId(worker::EntityId(1)) == FEntityId(worker::EntityId(1)));
  }

  UFUNCTION()
  void TestWorkerEntityIdEquality() const
  {
    T->TestTrue(TEXT("Equality with worker::EntityId"),
                FEntityId(worker::EntityId(1)) == worker::EntityId(1));
  }

  UFUNCTION()
  void TestInequality() const
  {
    T->TestTrue(TEXT("Inequality"),
                FEntityId(worker::EntityId(1)) != FEntityId(worker::EntityId(2)));
  }

  UFUNCTION(meta = (TestCase = "0,1000,10000"))
  void TestToString(const FString& Parameter) const
  {
    T->TestEqual(TEXT("ToString"), ToString(worker::EntityId(FCString::Atoi64(*Parameter))),
                 Parameter);
  }
};
