// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SpatialBasicAwaiter.h"
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialWorkerFlags.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAnyWorkerFlagUpdatedBP, const FString&, FlagName, const FString&, FlagValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnyWorkerFlagUpdated, const FString&, FlagName, const FString&, FlagValue);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnWorkerFlagUpdatedBP, const FString&, FlagName, const FString&, FlagValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkerFlagUpdated, const FString&, FlagName, const FString&, FlagValue);

UCLASS()
class SPATIALGDK_API USpatialWorkerFlags : public UObject
{
	GENERATED_BODY()

public:
	/** Gets value of a worker flag. Must be connected to SpatialOS to properly work.
	 * @param InFlagName - Name of worker flag
	 * @param OutFlagValue - Value of worker flag
	 * @return - If worker flag was found.
	 */
	bool GetWorkerFlag(const FString& InFlagName, FString& OutFlagValue) const;

	void SetWorkerFlag(const FString FlagName, FString FlagValue);

	void RemoveWorkerFlag(const FString FlagName);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAndInvokeAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

private:
	FOnAnyWorkerFlagUpdated OnAnyWorkerFlagUpdated;
	TMap<FString, FString> WorkerFlags;
	TMap<FString, FOnWorkerFlagUpdated> WorkerFlagCallbacks;
};
