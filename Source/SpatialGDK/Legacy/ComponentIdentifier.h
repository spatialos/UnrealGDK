// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineMinimal.h"
#include <improbable/worker.h>
#include "ComponentIdentifier.generated.h"

USTRUCT(BlueprintType)
struct FComponentIdentifier
{
  GENERATED_BODY()

  worker::EntityId EntityId;
  worker::ComponentId ComponentId;

  bool operator==(const FComponentIdentifier& Other) const
  {
    return (Other.EntityId == EntityId && Other.ComponentId == ComponentId);
  }

  FComponentIdentifier& operator=(const FComponentIdentifier& Other)
  {
    EntityId = Other.EntityId;
    ComponentId = Other.ComponentId;
    return *this;
  }

  friend uint32 GetTypeHash(worker::EntityId const& Rhs)
  {
    return HashEntityId(Rhs);
  }

  friend uint32 GetTypeHash(FComponentIdentifier const& Rhs)
  {
    // This creates a single integer value from ComponentId and EntityId using a Cantor pairing
    // function
    int64 a = Rhs.EntityId;
    int64 b = Rhs.ComponentId;

    int64 A = (a >= 0) ? (2 * a) : (-2 * a - 1);
    int64 B = (b >= 0) ? (2 * b) : (-2 * b - 1);

    int64 Res = (A >= B) ? (A * A + A + B) : (A + B * B);
    return HashEntityId(Res);
  }

  static uint32 HashEntityId(int64 x)
  {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return static_cast<uint32>(x);
  }
  static uint32 HashRequestId(uint32_t x)
  {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return static_cast<uint32>(x);
  }
};
