// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Internationalization/Text.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/LogMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/TextProperty.h"

#include "SpatialGDKSettings.h"

#include "SpatialStatics.generated.h"

class AActor;

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogSpatial, Log, All);

USTRUCT(BlueprintType)
struct FLockingToken
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SpatialGDK|Locking")
	int64 Token;
};

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
	 * Returns true if spatial networking and multi worker are enabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool IsSpatialMultiWorkerEnabled(const UObject* WorldContextObject);

	/**
	 * Returns true if there is more than one worker layer in the SpatialWorldSettings and IsMultiWorkerEnabled.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialOS|Offloading")
	static bool IsSpatialOffloadingEnabled(const UWorld* World);

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
	 * Functionally the same as the native Unreal PrintString but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable,
			  meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log print spatial",
					  AdvancedDisplay = "2", DevelopmentOnly),
			  Category = "Utilities|String")
	static void PrintStringSpatial(UObject* WorldContextObject, const FString& InString = FString(TEXT("Hello")),
								   bool bPrintToScreen = true, FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

	/**
	 * Functionally the same as the native Unreal PrintText but also logs to the spatial runtime.
	 */
	UFUNCTION(BlueprintCallable,
			  meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext, Keywords = "log spatial", AdvancedDisplay = "2",
					  DevelopmentOnly),
			  Category = "Utilities|Text")
	static void PrintTextSpatial(UObject* WorldContextObject, const FText InText = INVTEXT("Hello"), bool bPrintToScreen = true,
								 FLinearColor TextColor = FLinearColor(0.0, 0.66, 1.0), float Duration = 2.f);

	/**
	 * Returns true if worker flag with the given name was found.
	 * Gets value of a worker flag.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool GetWorkerFlag(const UObject* WorldContextObject, const FString& InFlagName, FString& OutFlagValue);

	/**
	 * Returns the Net Cull Distance distance/frequency pairs used in client qbi-f
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static TArray<FDistanceFrequencyPair> GetNCDDistanceRatios();

	/**
	 * Returns the full frequency net cull distance ratio used in client qbi-f
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static float GetFullFrequencyNetCullDistanceRatio();

	/**
	 * Returns the inspector colour for the given worker name.
	 * Argument expected in the form: UnrealWorker1a2s3d4f...
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static FColor GetInspectorColorForWorkerName(const FString& WorkerName);

	/**
	 * Returns the entity ID of a given actor, or 0 if we are not using spatial networking or Actor is nullptr.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpatialOS")
	static int64 GetActorEntityId(const AActor* Actor);

	/**
	 * Returns the entity ID as a string if the ID is valid, or "Invalid" if not
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpatialOS")
	static FString EntityIdToString(int64 EntityId);

	/**
	 * Returns the entity ID of a given actor as a string, or "Invalid" if we are not using spatial networking or Actor is nullptr.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpatialOS")
	static FString GetActorEntityIdAsString(const AActor* Actor);

	/**
	 * AcquireLock should only be called for an authoritative Actor from a server.
	 * If Spatial networking or multi-worker is disabled, this will return an invalid locking token.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static FLockingToken AcquireLock(AActor* Actor, const FString& DebugString = TEXT(""));

	/**
	 * ReleaseLock should only be called for an authoritative Actor from a server where the LockToken argument
	 * was previously returned from a call to AcquireLock.
	 * If Spatial networking or multi-worker is disabled, this will early.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static void ReleaseLock(const AActor* Actor, FLockingToken LockToken);

	/**
	 * IsLocked should only be called for an authoritative Actor from a server.
	 * If Spatial networking or multi-worker is disabled, this will early.
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialGDK|Locking")
	static bool IsLocked(const AActor* Actor);

	/**
	 * Returns the local layer name for this worker. Returns client worker type for all clients,
	 * and default layer for native servers.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static FName GetLayerName(const UObject* WorldContextObject);

private:
	static FName GetCurrentWorkerType(const UObject* WorldContext);
};
