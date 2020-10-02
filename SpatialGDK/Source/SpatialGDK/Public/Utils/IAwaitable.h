// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "UObject/Interface.h"

#include "IAwaitable.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAwaitable, Log, Log);

DECLARE_DELEGATE_OneParam(FOnReady, const FString& /* ErrorMessage */);

USTRUCT(BlueprintType)
struct FDelegateHandleBPWrapper
{
	GENERATED_BODY()

public:
	FDelegateHandle Handle;
};

UINTERFACE(MinimalAPI)
class UAwaitable : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class SPATIALGDK_API IAwaitable
{
	GENERATED_BODY()

public:
	virtual FDelegateHandle Await(const FOnReady& OnReadyDelegate, const float Timeout) = 0;
	virtual bool StopAwaiting(FDelegateHandle& Handle) = 0;

	DECLARE_EVENT(IAwaitable, FOnResetEvent);
	virtual FOnResetEvent& OnReset() = 0;
};
