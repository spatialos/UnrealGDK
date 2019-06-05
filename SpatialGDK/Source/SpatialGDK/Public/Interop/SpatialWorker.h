// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialWorker.generated.h"

/**
 * SpatialOS worker abstraction
 */
UCLASS()
class SPATIALGDK_API USpatialWorker : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Check whether the SpatialOS worker responsible for simulating the UWorld of the given actor can have authority over the actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool CanHaveAuthority(const AActor* Actor);
	
};
