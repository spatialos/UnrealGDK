// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SpatialWorkerFlags.generated.h"

UCLASS()
class SPATIALGDK_API USpatialWorkerFlags : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** Gets value of a worker flag. Must be connected to SpatialOS to properly work.
	 * @param Name - Name of worker flag
	 * @param Value - Value of worker flag
	 * @return - If worker flag was found.
	 */
	UFUNCTION(BlueprintCallable, Category="SpatialOS")
	static bool GetWorkerFlag(const FString& Name, FString& OutValue);

private:
	static void ApplyWorkerFlagUpdate(const struct Worker_FlagUpdateOp& Op);

	static TMap<FString, FString> WorkerFlags;

	friend class USpatialDispatcher;
};
