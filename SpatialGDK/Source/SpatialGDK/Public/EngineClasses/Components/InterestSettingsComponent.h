// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18915 - [IMPROVEMENT] InterestSettingsComponent
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InterestSettingsComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), NotSpatialType, Meta = (BlueprintSpawnableComponent))
class SPATIALGDK_API UInterestSettingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Whether or not to use NCD queries when calculating interest for our owner.")
	bool bUseNetCullDistanceSquaredForCheckoutRadius = true;
};
