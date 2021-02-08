// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialTestInitialOnlySpawnComponent.h"
#include "SpatialTestInitialOnlySpawnActorWithComponent.generated.h"

class USpatialTestInitialOnlySpawnComponent;

UCLASS()
class ASpatialTestInitialOnlySpawnActorWithComponent : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlySpawnActorWithComponent();

	// In the test, this Component is created and attached after spawning this Actor.
	UPROPERTY(Replicated)
	USpatialTestInitialOnlySpawnComponent* OnSpawnComponent;

	// In the test, this Component is created and attached as part of PostInitializeComponents.
	UPROPERTY(Replicated)
	USpatialTestInitialOnlySpawnComponent* PostInitializeComponent;

	// In the test, this Component is created and attached one second after spawning this Actor.
	UPROPERTY(Replicated)
	USpatialTestInitialOnlySpawnComponent* LateAddedComponent;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;
};
