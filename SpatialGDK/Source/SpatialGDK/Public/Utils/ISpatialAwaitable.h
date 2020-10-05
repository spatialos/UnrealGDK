// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "UObject/Interface.h"

#include "ISpatialAwaitable.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAwaitable, Log, Log);

DECLARE_DELEGATE_OneParam(FOnReady, const FString& /* ErrorMessage */);

USTRUCT(BlueprintType)
struct FSpatialAwaitableDelegateHandleBPWrapper
{
	GENERATED_BODY()

public:
	FDelegateHandle Handle;
};

UINTERFACE(MinimalAPI)
class USpatialAwaitable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Generic awaitable interface.
 */
class SPATIALGDK_API ISpatialAwaitable
{
	GENERATED_BODY()

public:
	virtual FDelegateHandle Await(const FOnReady& OnReadyDelegate, const float Timeout) = 0;
	virtual bool StopAwaiting(FDelegateHandle& Handle) = 0;

	DECLARE_EVENT(ISpatialAwaitable, FSpatialAwaitableOnResetEvent);
	virtual FSpatialAwaitableOnResetEvent& OnReset() = 0;
};
