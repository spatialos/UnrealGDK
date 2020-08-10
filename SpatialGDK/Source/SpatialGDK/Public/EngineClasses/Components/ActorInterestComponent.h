// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Interop/SpatialInterestConstraints.h"
#include "Schema/Interest.h"

#include "ActorInterestComponent.generated.h"

class USpatialClassInfoManager;

/**
 * Creates a set of SpatialOS Queries for describing interest that this actor has in other entities.
 */
UCLASS(ClassGroup = (SpatialGDK), NotSpatialType, Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API UActorInterestComponent final : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorInterestComponent() = default;
	~UActorInterestComponent() = default;

	void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
										   SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const;

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
