// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpatialGameState.generated.h"

class ASpatialMetricsDisplay;

/**
 * 
 */
UCLASS()
class SPATIALGDK_API ASpatialGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASpatialGameState();

	UFUNCTION(Exec)
	void SpatialToggleStatDisplay();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = Classes)
	TSubclassOf<ASpatialMetricsDisplay> SpatialMetricsDisplayClass;

private:
	void SpawnSpatialMetricsDisplay();

	UPROPERTY(Replicated)
	ASpatialMetricsDisplay* SpatialMetricsDisplay;
};
