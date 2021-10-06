// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "SpatialTestInitialOnlySpawnComponent.generated.h"

UCLASS()
class USpatialTestInitialOnlySpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialTestInitialOnlySpawnComponent();

	UPROPERTY(Replicated)
	int Int_Initial = 1;

	UPROPERTY(Replicated)
	int Int_Replicate = 1;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
