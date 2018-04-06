// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "ComponentIdentifier.h"
#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"

#include "EntityId.generated.h"

USTRUCT(BlueprintType)
struct SPATIALOS_API FEntityId
{
  GENERATED_USTRUCT_BODY()

public:
  FORCEINLINE FEntityId()
  {
    Underlying = 0;
  }

  FORCEINLINE FEntityId(const FEntityId& InEntityId)
  {
    Underlying = InEntityId.Underlying;
  }

  FORCEINLINE FEntityId(const worker::EntityId& InEntityId)
  {
    Underlying = InEntityId;
  }

  FORCEINLINE FEntityId& operator=(const FEntityId& Other)
  {
    Underlying = Other.Underlying;
    return *this;
  }

  FORCEINLINE bool operator==(const FEntityId& Other) const
  {
    return Underlying == Other.Underlying;
  }

  FORCEINLINE bool operator==(const worker::EntityId& Other) const
  {
    return Underlying == Other;
  }

  FORCEINLINE bool operator!=(const FEntityId& Other) const
  {
    return Underlying != Other.Underlying;
  }

  worker::EntityId ToSpatialEntityId() const
  {
    return Underlying;
  }

private:
  worker::EntityId Underlying;

  friend uint32 GetTypeHash(FEntityId const& Rhs)
  {
    return FComponentIdentifier::HashEntityId(Rhs.ToSpatialEntityId());
  }
};

/**
 * Format a SpatialOS EntityId as a string.
*/
inline FString ToString(const worker::EntityId& EntityId)
{
  return FString::Printf(TEXT("%lld"), EntityId);
}