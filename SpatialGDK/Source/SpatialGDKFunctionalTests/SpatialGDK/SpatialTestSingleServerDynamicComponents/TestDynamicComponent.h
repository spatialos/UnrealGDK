// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
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

	UPROPERTY(Replicated)
	int32 OwnerOnlyReplicatedVar;

	UPROPERTY(Replicated)
	int32 InitialOnlyReplicatedVar;

	UPROPERTY(Handover)
	int32 HandoverReplicatedVar;
};
