// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "ISpatialAwaitable.h"
#include "UObject/NoExportTypes.h"

#include "SpatialBasicAwaiter.generated.h"

DECLARE_DERIVED_EVENT(USpatialBasicAwaiter, ISpatialAwaitable::FSpatialAwaitableOnResetEvent, FSpatialAwaitableOnResetEvent);

/**
 * Object to await a single condition becoming true.
 * If the awaiter is not ready yet, delegates passed via Await are stored, and called once the awaiter becomes ready (by having Ready called
 * on it). Once the awaiter is ready, any delegate passed via Await will be called immediately. A common example of a condition would be an
 * object being ready for use by other objects.
 */
UCLASS(Blueprintable, DefaultToInstanced)
class SPATIALGDK_API USpatialBasicAwaiter : public UObject, public ISpatialAwaitable
{
	GENERATED_BODY()

public:
	virtual ~USpatialBasicAwaiter() {}

	/** ISpatialAwaitable Implementation */
	// A Timeout value of 0 means the await should never time out.
	virtual FDelegateHandle Await(const FOnReady& OnReadyDelegate, const float Timeout = 0.f) override;
	virtual bool StopAwaiting(FDelegateHandle& Handle) override;
	virtual FSpatialAwaitableOnResetEvent& OnReset() override;
	/** End ISpatialAwaitable Implementation */

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = Awaiter)
	void Ready();

	UFUNCTION(BlueprintCallable, Category = Awaiter)
	void Reset();

	static const FString TIMEOUT_MESSAGE;

protected:
	DECLARE_EVENT_OneParam(USpatialBasicAwaiter, FOnReadyEvent, const FString&);

	FOnReadyEvent OnReadyEvent;

private:
	bool bIsReady;

	void InvokeQueuedDelegates(const FString& ErrorStatus = FString{});
	FSpatialAwaitableOnResetEvent OnResetEvent;

	TSet<FTimerHandle> TimeoutHandles;
};
