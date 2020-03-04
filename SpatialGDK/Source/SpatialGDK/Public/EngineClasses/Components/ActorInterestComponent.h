// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interop/SpatialInterestConstraints.h"

#include "ActorInterestComponent.generated.h"

namespace SpatialGDK
{
struct QueryConstraint;
}
class USpatialClassInfoManager;

/**
 * Creates a set of SpatialOS Queries for describing interest that this actor has in other entities.
 */
UCLASS(ClassGroup=(SpatialGDK), NotSpatialType, Meta=(BlueprintSpawnableComponent))
class SPATIALGDK_API UActorInterestComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorInterestComponent() = default;
	~UActorInterestComponent() = default;

	void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager, TMap<float, TArray<SpatialGDK::QueryConstraint>>& OutFrequencyToQueryConstraints) const;

	/**
	 * Whether to use NetCullDistanceSquared to generate constraints relative to the Actor that this component is attached to.
	 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Interest")
	bool bUseNetCullDistanceSquaredForCheckoutRadius = true;

	/**
	 * The Queries associated with this component.
	 */
	UPROPERTY(BlueprintReadonly, EditDefaultsOnly, Category = "Interest")
	TArray<FQueryData> Queries;

};
