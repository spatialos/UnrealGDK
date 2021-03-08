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

	UPROPERTY(Replicated)
	USpatialTestInitialOnlySpawnComponent* InitialOnlyComponent;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
