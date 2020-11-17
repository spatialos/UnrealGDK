// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "SpatialWorkerFlags.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAnyWorkerFlagUpdatedBP, const FString&, FlagName, const FString&, FlagValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnyWorkerFlagUpdated, const FString&, FlagName, const FString&, FlagValue);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWorkerFlagUpdatedBP, const FString&, FlagValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorkerFlagUpdated, const FString&, FlagValue);

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

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAndInvokeAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void UnregisterAnyFlagUpdatedCallback(const FOnAnyWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterAndInvokeFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void UnregisterFlagUpdatedCallback(const FString& InFlagName, const FOnWorkerFlagUpdatedBP& InDelegate);

	void ApplyWorkerFlagUpdate(const Worker_FlagUpdateOp& Op);

private:
	FOnAnyWorkerFlagUpdated OnAnyWorkerFlagUpdated;
	TMap<FString, FOnWorkerFlagUpdated> OnWorkerFlagUpdatedMap;
	TMap<FString, FString> WorkerFlags;
};
