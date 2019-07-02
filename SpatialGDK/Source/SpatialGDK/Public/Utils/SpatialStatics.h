// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialStatics.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStatics, Log, All);

UCLASS()
class SPATIALGDK_API USpatialStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Returns whether SpatialOS networking has been enabled or not.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialOS")
    static bool SpatialNetworkingEnabled();

    /**
     * Returns true iff SpatialOS networking AND offloading are both enabled.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialOS")
    static bool SpatialOffloadingEnabled();

    /**
     * If SpatialOS networking is disabled, or if SpatialOS networking is enabled but offloading is disabled,
     * this will return Actor::HasAuthority(). If SpatialOS networking AND offloading is enabled, this will
     * returns true iff actor group worker association matches the current worker type.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialOS")
    static bool IsActorGroupOwner(const AActor* Actor);
};
