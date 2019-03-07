// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SimulatedPlayer.generated.h"


UCLASS()
class SPATIALGDK_API USimulatedPlayer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Get whether we are a simulated client. */
	UFUNCTION(BlueprintPure)
	static bool IsSimulatedPlayer();
};
