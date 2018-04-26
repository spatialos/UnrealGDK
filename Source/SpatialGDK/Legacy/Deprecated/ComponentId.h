// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "ComponentId.generated.h"
#include "ComponentIdentifier.h"
#include "CoreMinimal.h"
#include "SpatialGDKWorkerTypes.h"

USTRUCT(BlueprintType)
struct SPATIALGDK_API FComponentId
{
	GENERATED_USTRUCT_BODY()

public:
	FORCEINLINE FComponentId()
	{
		Underlying = 0;
	}

	FORCEINLINE FComponentId(const FComponentId &InComponentId)
	{
		Underlying = InComponentId.Underlying;
	}

	FORCEINLINE FComponentId(const worker::ComponentId &InComponentId)
	{
		Underlying = InComponentId;
	}

	FORCEINLINE FComponentId &operator=(const FComponentId &Other)
	{
		Underlying = Other.Underlying;
		return *this;
	}

	FORCEINLINE bool operator==(const FComponentId &Other) const
	{
		return Underlying == Other.Underlying;
	}

	FORCEINLINE bool operator==(const worker::ComponentId &Other) const
	{
		return Underlying == Other;
	}

	FORCEINLINE bool operator!=(const FComponentId &Other) const
	{
		return Underlying != Other.Underlying;
	}

	worker::ComponentId ToSpatialComponentId() const
	{
		return Underlying;
	}

private:
	worker::ComponentId Underlying;

	friend uint32 GetTypeHash(FComponentId const &Rhs)
	{
		return FComponentIdentifier::HashEntityId(Rhs.ToSpatialComponentId());
	}
};

/**
* Format a SpatialOS ComponentId as a string.
*/
inline FString ToString(const worker::ComponentId &ComponentId)
{
	return FString::Printf(TEXT("%dll"), ComponentId);
}
