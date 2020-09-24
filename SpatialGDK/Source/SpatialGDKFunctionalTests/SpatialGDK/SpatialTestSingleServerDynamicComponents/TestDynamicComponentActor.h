// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestDynamicComponentActor.generated.h"

class UTestDynamicComponent;

/**
 * A replicated, always relevant Actor used in SpatialTestSingleServerDynamicComponents to test dynamic components.
 * Contains 3 replicated dynamic components attached at various points in its lifecycle.
 */

UCLASS()
class ATestDynamicComponentActor : public AActor
{
	GENERATED_BODY()

public:
	ATestDynamicComponentActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// In the test, this Component is created and attached after spawning the ATestDynamicComponentActor.
	UPROPERTY(Replicated)
	UTestDynamicComponent* OnSpawnComponent;

	// In the test, this Component is created and attached as part of PostInitializeComponents.
	UPROPERTY(Replicated)
	UTestDynamicComponent* PostInitializeComponent;

	// In the test, this Component is created and attached one second after spawning the ATestDynamicComponentActor.
	UPROPERTY(Replicated)
	UTestDynamicComponent* LateAddedComponent;

	virtual void PostInitializeComponents() override;
};
