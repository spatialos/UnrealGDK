// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "BasicAwaiter.h"

#include "Engine/World.h"
#include "TimerManager.h"

const FString UBasicAwaiter::TIMEOUT_MESSAGE = FString{ TEXT("Timed out while waiting to become Ready!") };
constexpr TCHAR DESTROYED_MESSAGE_TEMPLATE[] = TEXT("Awaiter '%s' was destroyed!");

void UBasicAwaiter::BeginDestroy()
{
	InvokeQueuedDelegates(FString::Printf(DESTROYED_MESSAGE_TEMPLATE, *GetName()));
	Super::BeginDestroy();
}

FDelegateHandle UBasicAwaiter::Await(const FOnReady& OnReadyDelegate, const float Timeout)
{
	if (bIsReady)
	{
		OnReadyDelegate.ExecuteIfBound(FString{});
		return FDelegateHandle{};
	}
	else
	{
		FDelegateHandle OutHandle = OnReadyEvent.Add(OnReadyDelegate);

		if (Timeout > 0.f)
		{
			if (UWorld* World = GetWorld())
			{
				FTimerManager& TimerManager = World->GetTimerManager();
				FTimerHandle Handle;

				TimerManager.SetTimer(Handle,
									  FTimerDelegate::CreateWeakLambda(this,
																	   [this, OutHandle, OnReadyDelegate]() {
																		   if (OnReadyDelegate.IsBound())
																		   {
																			   OnReadyDelegate.Execute(TIMEOUT_MESSAGE);
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

bool UBasicAwaiter::StopAwaiting(FDelegateHandle& Handle)
{
	return OnReadyEvent.Remove(Handle);
}

IAwaitable::FOnResetEvent& UBasicAwaiter::OnReset()
{
	return OnResetEvent;
}

void UBasicAwaiter::Ready()
{
	// Early exit if already ready
	if (bIsReady)
	{
		return;
	}

	bIsReady = true;
	InvokeQueuedDelegates();
}

void UBasicAwaiter::Reset()
{
	if (!bIsReady)
	{
		return;
	}

	bIsReady = false;
	OnResetEvent.Broadcast();
	OnResetEvent.Clear();
}

void UBasicAwaiter::InvokeQueuedDelegates(const FString& ErrorStatus /* = FString */)
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
