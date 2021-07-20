// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "Interfaces/ISpatialInterestProvider.h"
#include "AbstractInterestComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), abstract, NotSpatialType)
class SPATIALGDK_API UAbstractInterestComponent : public UActorComponent, public ISpatialInterestProvider
{
	GENERATED_BODY()

public:
	UAbstractInterestComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { bIsUpdateRequired = false; }

	virtual void SetIsUpdateRequired(const bool bUpdateRequired) override { bIsUpdateRequired = bUpdateRequired; }
	virtual bool IsUpdateRequired() const override { return bIsUpdateRequired; }

	virtual bool GetUseNetCullDistanceSquaredForCheckoutRadius() const override { return bUseNetCullDistanceSquaredForCheckoutRadius; }

public:
	/**
	 * Whether to use NetCullDistanceSquared to generate constraints relative to the Actor that this component is attached to.
	*/
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Interest")
	bool bUseNetCullDistanceSquaredForCheckoutRadius = true;

private:
	bool bIsUpdateRequired;
};
