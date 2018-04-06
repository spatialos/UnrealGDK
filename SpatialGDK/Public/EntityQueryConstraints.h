#pragma once

#include "ComponentId.h"
#include "CoreMinimal.h"
#include "EntityId.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/collections.h"
#include "EntityQueryConstraints.generated.h"

/**
* Struct defining the result of an AND or OR Entity query constraint
*/

UCLASS(Blueprintable, NotPlaceable)
class SPATIALOS_API UEntityQueryConstraint : public UObject
{
  GENERATED_BODY()

public:
  UEntityQueryConstraint()
  {
  }

  worker::Option<worker::query::Constraint> Underlying;
};

/**
* Struct defining Entity ID query constraint
*/

USTRUCT(BlueprintType)
struct SPATIALOS_API FEntityIdQueryConstraint
{
  GENERATED_BODY()

public:
  FORCEINLINE FEntityIdQueryConstraint()
  {
  }

  FORCEINLINE FEntityIdQueryConstraint(const FEntityId& InEntityId)
  {
    Underlying = InEntityId;
  }

  FORCEINLINE FEntityIdQueryConstraint(int InEntityId)
  {
    Underlying = FEntityId(static_cast<worker::EntityId>(InEntityId));
  }

  FEntityId Underlying;
};

/**
* Struct defining Component ID query constraint
*/
USTRUCT(BlueprintType)
struct SPATIALOS_API FComponentIdQueryConstraint
{
  GENERATED_BODY()

public:
  FORCEINLINE FComponentIdQueryConstraint()
  {
  }

  FORCEINLINE FComponentIdQueryConstraint(const FComponentId& InComponentId)
  {
    Underlying = InComponentId;
  }

  FORCEINLINE FComponentIdQueryConstraint(int InComponentId)
  {
    Underlying = FComponentId(static_cast<worker::ComponentId>(InComponentId));
  }

  FComponentId Underlying;
};

/**
* Struct defining sphere query constraint
*/

USTRUCT(BlueprintType)
struct SPATIALOS_API FSphereQueryConstraint
{
  GENERATED_BODY()

  FORCEINLINE FSphereQueryConstraint()
  {
  }

  FORCEINLINE FSphereQueryConstraint(const FVector& InPosition, float InRadius)
  {
    Position = InPosition;
    Radius = InRadius;
  }

  FVector Position;
  float Radius;
};
