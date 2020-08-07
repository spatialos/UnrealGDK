// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTestWorkerDelegationComponent.generated.h"

/*
 * Actor Component for Functional Testing purposes only that allows you to delegate its Actor to a specific Server Worker.
 * Note that currently this functionality only works in single layer Load Balancing Strategies, and your Default Load
 * Balancing Strategy needs to implement ISpatialFunctionalTestLBDelegationInterface.
 */
UCLASS(BlueprintType)
class SPATIALGDKFUNCTIONALTESTS_API USpatialFunctionalTestWorkerDelegationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpatialFunctionalTestWorkerDelegationComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Default")
	int WorkerId = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Default")
	bool bIsPersistent = false;
};
