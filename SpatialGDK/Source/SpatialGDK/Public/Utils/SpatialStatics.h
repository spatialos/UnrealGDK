// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "SpatialStatics.generated.h"

class AActor;

UCLASS()
class SPATIALGDK_API USpatialStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

	/**
	 * Returns true if SpatialOS Networking is enabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS")
	static bool IsSpatialNetworkingEnabled();

       /**
        * Returns true if SpatialOS Offloading is enabled.
        */
       UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading")
       static bool IsSpatialOffloadingEnabled();

	/**
	 * Returns true if the current Worker Type owns the Actor Group this Actor belongs to.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading")
	static bool IsActorGroupOwnerForActor(const AActor* Actor);

	/**
	 * Returns true if the current Worker Type owns the Actor Group this Actor Class belongs to.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading", meta = (WorldContext = "WorldContextObject"))
	static bool IsActorGroupOwnerForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass);

	/**
	 * Returns true if the current Worker Type owns this Actor Group.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading", meta = (WorldContext = "WorldContextObject"))
	static bool IsActorGroupOwner(const UObject* WorldContextObject, const FName ActorGroup);

	/**
	 * Returns the ActorGroup this Actor belongs to.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading")
	static FName GetActorGroupForActor(const AActor* Actor);

	/**
	 * Returns the ActorGroup this Actor Class belongs to.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading", meta = (WorldContext = "WorldContextObject"))
	static FName GetActorGroupForClass(const UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass);

private:

	static class UActorGroupManager* GetActorGroupManager(const UObject* WorldContext);
	static FName GetCurrentWorkerType(const UObject* WorldContext);
};
