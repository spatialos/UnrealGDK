// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialStatics.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStatics, Log, All);

UCLASS()
class SPATIALGDK_API USpatialStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

private:
	
	static bool IsSpatialOffloadingEnabled();

public:

	/**
	 * Returns true if SpatialOS Networking is enabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS")
	static bool IsSpatialNetworkingEnabled();

	/**
     * Returns true if the current Worker Type owns the Actor Group this Actor belongs to.
	 * Equivalent to HasAuthority when Spatial Networking is disabled.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialOS")
    static bool IsActorGroupOwnerForActor(const AActor* Actor);

	/**
	 * Returns true if the current Worker Type owns the Actor Group this Actor Class belongs to.
	 * Equivalent to HasAuthority when Spatial Networking is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool IsActorGroupOwnerForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass);

	/**
	 * Returns true if the current Worker Type owns this Actor Group.
	 * Equivalent to HasAuthority when Spatial Networking is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool IsActorGroupOwner(const UObject* WorldContextObject, const FName ActorGroup);
};
