// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LockingStatics.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocking, Log, All);

USTRUCT(BlueprintType)
struct FLockingToken
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 Token;
};

UCLASS()
class SPATIALGDK_API ULockingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static FLockingToken AcquireLock(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "SpatialGDK|Locking")
	static void ReleaseLock(const AActor* Actor, FLockingToken LockToken);

	UFUNCTION(BlueprintPure, Category = "SpatialGDK|Locking")
	static bool IsLocked(const AActor* Actor);
};
