// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Internationalization/Text.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/TextProperty.h"

#include "SpatialStatics.generated.h"

class AActor;

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatial, Log, All);

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

	/**
	 * Functionally the same as the native Unreal PrintString but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print spatial", AdvancedDisplay = "2", DevelopmentOnly), Category = "Utilities|String")
	static void PrintStringSpatial(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")), bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

	/**
	 * Functionally the same as the native Unreal PrintText but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log spatial", AdvancedDisplay = "2", DevelopmentOnly), Category = "Utilities|Text")
	static void PrintTextSpatial(UObject* WorldContextObject, const FText InText = INVTEXT("Hello"), bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

private:

	static class UActorGroupManager* GetActorGroupManager(const UObject* WorldContext);
	static FName GetCurrentWorkerType(const UObject* WorldContext);
};
