// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "EngineClasses/Components/AbstractInterestComponent.h"
#include "Interop/SpatialInterestConstraints.h"
#include "Schema/Interest.h"

#include "ActorInterestComponent.generated.h"

class USpatialClassInfoManager;

/**
 * Creates a set of SpatialOS Queries for describing interest that this actor has in other entities.
 */
UCLASS(ClassGroup = (SpatialGDK), NotSpatialType, Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API UActorInterestComponent final : public UAbstractInterestComponent
{
	GENERATED_BODY()

public:
	UActorInterestComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
										   SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const override;
	/**
	 * The Queries associated with this component.
	 */
	UPROPERTY(BlueprintReadonly, EditDefaultsOnly, Category = "Interest")
	TArray<FQueryData> Queries;
};
