// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
#include "EngineClasses/Components/AbstractInterestComponent.h"
// NWX_END
#include "Interop/SpatialInterestConstraints.h"
#include "Schema/Interest.h"

#include "ActorInterestComponent.generated.h"

class USpatialClassInfoManager;

/**
 * Creates a set of SpatialOS Queries for describing interest that this actor has in other entities.
 */
UCLASS(ClassGroup = (SpatialGDK), NotSpatialType, Meta = (BlueprintSpawnableComponent))
// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
//class SPATIALGDK_API UActorInterestComponent final : public UActorComponent
class SPATIALGDK_API UActorInterestComponent final : public UAbstractInterestComponent
// NWX_END
{
	GENERATED_BODY()

public:
	// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
	//UActorInterestComponent() = default;
	//~UActorInterestComponent() = default;
	//void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
	//									   SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const;
	UActorInterestComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
										   SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const override;
	// NWX_END

	/**
	 * Whether to use NetCullDistanceSquared to generate constraints relative to the Actor that this component is attached to.
	 */
	// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
	//UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Interest")
	//bool bUseNetCullDistanceSquaredForCheckoutRadius = true;
	// NWX_END
	/**
	 * The Queries associated with this component.
	 */
	UPROPERTY(BlueprintReadonly, EditDefaultsOnly, Category = "Interest")
	TArray<FQueryData> Queries;
};
