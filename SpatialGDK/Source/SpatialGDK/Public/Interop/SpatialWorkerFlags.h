// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include <WorkerSDK/improbable/c_worker.h>
#include "SpatialWorkerFlags.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnWorkerFlagsUpdatedBP, FString, FlagName, FString, FlagValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkerFlagsUpdated, FString, FlagName, FString, FlagValue);

UCLASS()
class SPATIALGDK_API USpatialWorkerFlags : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Gets value of a worker flag. Must be connected to SpatialOS to properly work.
	 * @param Name - Name of worker flag
	 * @param OutValue - Value of worker flag
	 * @return - If worker flag was found.
	 */
	UFUNCTION(BlueprintCallable, Category="SpatialOS")
	static bool GetWorkerFlag(const FString& Name, FString& OutValue);
	
	static FOnWorkerFlagsUpdated& GetOnWorkerFlagsUpdated();
	
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void BindToOnWorkerFlagsUpdated(const FOnWorkerFlagsUpdatedBP& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void UnbindFromOnWorkerFlagsUpdated(const FOnWorkerFlagsUpdatedBP& InDelegate);

	static FOnWorkerFlagsUpdated OnWorkerFlagsUpdated;
private:
	static void ApplyWorkerFlagUpdate(const Worker_FlagUpdateOp& Op);

	static TMap<FString, FString> WorkerFlags;

	friend class USpatialDispatcher;
};
