// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/SpatialPingComponent.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogSpatialPingComponent);

USpatialPingComponent::USpatialPingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bReplicates = true;
}

void USpatialPingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialPingComponent, ReplicatedPingID);
}

void USpatialPingComponent::BeginPlay()
{
	Super::BeginPlay();
	// Attempt to cast the owner of this component to the PlayerController class.
	// Component will do nothing if cast fails.
	OwningController = Cast<APlayerController>(GetOwner());
	if (OwningController == nullptr)
	{
		UE_LOG(LogSpatialPingComponent, Warning, TEXT("SpatialPingComponent did not find a valid owning PlayerController and will not function correctly. Ensure this component is only attached to a PlayerController."));
	}

	if (bStartWithPingEnabled)
	{
		SetPingEnabled(true);
	}
}

void USpatialPingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Clear up the timer before the component is disabled or destroyed.
	SetPingEnabled(false);
}

bool USpatialPingComponent::GetIsPingEnabled() const
{
	return bIsPingEnabled;
}

void USpatialPingComponent::SetPingEnabled(bool bSetEnabled)
{
	// Only execute if this component is attached to a player controller.
	if (OwningController != nullptr)
	{
		// Only execute on owning local client.
		if (OwningController->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
		{
			if (bSetEnabled && !bIsPingEnabled)
			{
				EnablePing();
			}
			else if (!bSetEnabled && bIsPingEnabled)
			{
				DisablePing();
			}
		}
	}
}

float USpatialPingComponent::GetPing() const
{
	return RoundTripPing;
}

void USpatialPingComponent::EnablePing()
{
	if (UWorld* World = GetWorld())
	{
		LastSentPingID = 0;
		TimeoutCount = 0;
		// Send a new ping, which will trigger a self-perpetuating sequence via timers.
		SendNewPing();
		// Set looping timer to 'tick' this component, it doesn't send any pings but matches the MinPingInterval for passing updates to the owning controller.
		World->GetTimerManager().SetTimer(PingTickHandle, this, &USpatialPingComponent::TickPingComponent, MinPingInterval, true);
		bIsPingEnabled = true;
	}
}

void USpatialPingComponent::DisablePing()
{
	if (UWorld* World = GetWorld())
	{
		bIsPingEnabled = false;
		// Clear the timers.
		World->GetTimerManager().ClearTimer(PingTickHandle);
		World->GetTimerManager().ClearTimer(PingTimerHandle);
		// Reset ping output value.
		RoundTripPing = 0.f;
	}
}

void USpatialPingComponent::TickPingComponent()
{
	// Pass latest measured ping to owning controller to be processed by PlayerState.
	OwningController->UpdatePing(RoundTripPing);
}

void USpatialPingComponent::SendNewPing()
{
	// Generate a new ping ID.
	uint16 NewPingID = LastSentPingID + 1;
	// Send and record new ping ID.
	SendServerWorkerPingID(NewPingID);
	LastSentPingID = NewPingID;
	LastSentPingTimestamp = FPlatformTime::Seconds();
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// Set a timeout timer to await a reply.
		World->GetTimerManager().SetTimer(PingTimerHandle, this, &USpatialPingComponent::OnPingTimeout, TimeoutLimit, false);
	}
}

void USpatialPingComponent::OnPingTimeout()
{
	// Update round trip ping to reflect timeout.
	TimeoutCount++;
	RoundTripPing = TimeoutLimit * TimeoutCount;
	// Attempt to send another ping.
	SendNewPing();
}

void USpatialPingComponent::OnRep_ReplicatedPingID()
{
	// Check that replicated ping ID matches the last sent AND check ping is enabled to catch cases where a value is replicated after being locally disabled.
	if (ReplicatedPingID == LastSentPingID && bIsPingEnabled)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			// Clear the timeout timer.
			World->GetTimerManager().ClearTimer(PingTimerHandle);
			TimeoutCount = 0;
		}

		// Calculate the round trip ping
		RoundTripPing = static_cast<float>(FPlatformTime::Seconds() - LastSentPingTimestamp);
		if (RoundTripPing >= MinPingInterval)
		{
			// If the current ping exceeds the min interval then send a new ping immediately.
			SendNewPing();
		}
		else
		{
			// Otherwise set a timer for the remaining interval before sending another.
			float RemainingInterval = MinPingInterval - RoundTripPing;
			if (World != nullptr)
			{
				World->GetTimerManager().SetTimer(PingTimerHandle, this, &USpatialPingComponent::SendNewPing, RemainingInterval, false);
			}
		}
	}
}

bool USpatialPingComponent::SendServerWorkerPingID_Validate(uint16 PingID)
{
	return true;
}

void USpatialPingComponent::SendServerWorkerPingID_Implementation(uint16 PingID)
{
	ReplicatedPingID = PingID;
	GetOwner()->ForceNetUpdate();
}
