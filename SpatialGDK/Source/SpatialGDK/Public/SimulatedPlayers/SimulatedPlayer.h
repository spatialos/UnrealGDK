// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimulatedPlayer.generated.h"

UCLASS()
class SPATIALGDK_API USimulatedPlayer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get whether this client is simulated.
	 * This will return true for clients launched inside simulated player deployments,
	 * or simulated clients launched from the Editor.
	 */
	UFUNCTION(BlueprintPure, Category="SpatialOS|SimulatedPlayer", meta = (WorldContext = WorldContextObject))
	static bool IsSimulatedPlayer(UObject* WorldContextObject);
};
