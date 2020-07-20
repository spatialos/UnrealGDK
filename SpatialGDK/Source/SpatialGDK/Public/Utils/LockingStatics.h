// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LockingStatics.generated.h"

// This log category will always log to the spatial runtime and thus also be printed in the SpatialOutput.
DECLARE_LOG_CATEGORY_EXTERN(LogLocking, Log, All);

UCLASS()
class SPATIALGDK_API ULockingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static int64 AcquireLock(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static void ReleaseLock(const AActor* Actor, int64 LockToken);

	UFUNCTION(BlueprintPure, Category = "SpatialGDK|Locking")
	static bool IsLocked(const AActor* Actor);
};
