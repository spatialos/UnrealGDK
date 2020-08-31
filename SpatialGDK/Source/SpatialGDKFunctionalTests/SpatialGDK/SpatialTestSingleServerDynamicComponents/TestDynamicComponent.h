// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TestDynamicComponent.generated.h"

/**
 * Simple replicated component with a replicated array of references, used in SpatialTestSingleServerDynamicComponents
 */
UCLASS()
class UTestDynamicComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTestDynamicComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<AActor*> ReferencesArray;
};
