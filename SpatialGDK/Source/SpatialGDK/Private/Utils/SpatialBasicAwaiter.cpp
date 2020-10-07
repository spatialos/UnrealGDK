// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialBasicAwaiter.h"

#include "Engine/World.h"
#include "TimerManager.h"

void USpatialBasicAwaiter::BeginDestroy()
{
	InvokeQueuedDelegates(FString::Printf(TEXT("Awaiter '%s' was destroyed!"), *GetName()));
	Super::BeginDestroy();
}

FDelegateHandle USpatialBasicAwaiter::Await(const FOnReady& OnReadyDelegate, const float Timeout)
{
	// We already became ready in the past, so the delegate can be executed immediately
	if (bIsReady)
	{
		OnReadyDelegate.ExecuteIfBound(FString{});
		return FDelegateHandle{};
	}
	// Still waiting to become ready, so store the delegate to be executed when we eventually do become ready
	else
	{
		FDelegateHandle OutHandle = OnReadyEvent.Add(OnReadyDelegate);

		// If requested to, add a timer to remove the delegate in case becoming ready takes too long
		if (Timeout > 0.f)
		{
			if (UWorld* World = GetWorld())
			{
				FTimerManager& TimerManager = World->GetTimerManager();
				FTimerHandle Handle;

				TimerManager.SetTimer(Handle,
									  FTimerDelegate::CreateWeakLambda(
										  this,
										  [this, OutHandle, OnReadyDelegate]() {
											  if (OnReadyDelegate.IsBound())
											  {
												  OnReadyDelegate.Execute(FString{ TEXT("Timed out while waiting to become Ready!") });
												  OnReadyEvent.Remove(OutHandle);
											  }
										  }),
									  Timeout, false);

				TimeoutHandles.Add(Handle);
			}
			else
			{
				UE_LOG(LogAwaitable, Error,
					   TEXT("UBasicAwaiter::Await could not find a valid UWorld reference! (Could not set up timeout timer)"));
			}
		}

		return OutHandle;
	}
}

bool USpatialBasicAwaiter::StopAwaiting(FDelegateHandle& Handle)
{
	return OnReadyEvent.Remove(Handle);
}

ISpatialAwaitable::FSpatialAwaitableOnResetEvent& USpatialBasicAwaiter::OnReset()
{
	return OnResetEvent;
}

void USpatialBasicAwaiter::Ready()
{
	// Early exit if already ready
	if (bIsReady)
	{
		return;
	}

	bIsReady = true;
	InvokeQueuedDelegates();
}

void USpatialBasicAwaiter::Reset()
{
	if (!bIsReady)
	{
		return;
	}

	bIsReady = false;
	OnResetEvent.Broadcast();
	OnResetEvent.Clear();
}

void USpatialBasicAwaiter::InvokeQueuedDelegates(const FString& ErrorStatus /* = FString{} */)
{
	OnReadyEvent.Broadcast(ErrorStatus);
	OnReadyEvent.Clear();

	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& TimeoutHandle : TimeoutHandles)
		{
			World->GetTimerManager().ClearTimer(TimeoutHandle);
		}
	}
}
