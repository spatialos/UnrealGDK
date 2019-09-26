// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/SpatialPingComponent.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

USpatialPingComponent::USpatialPingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bReplicates = true;

	bStartWithPingEnabled = true;
}

void USpatialPingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialPingComponent, ReplicatedPingTimestamp);
}

void USpatialPingComponent::BeginPlay()
{
	Super::BeginPlay();
	// Attempt to cast the owner of this component to the PlayerController class.
	// Component will do nothing if cast fails.
	OwningController = Cast<APlayerController>(GetOwner());
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
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// Set looping timer to 'tick' this component and send ping RPC with configurable frequency.
		World->GetTimerManager().SetTimer(PingTimerHandle, this, &USpatialPingComponent::TickPingComponent, PingFrequency, true);
		bIsPingEnabled = true;
	}
}

void USpatialPingComponent::DisablePing()
{
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		// Clear the timer.
		World->GetTimerManager().ClearTimer(PingTimerHandle);
		bIsPingEnabled = false;
		RoundTripPing = 0.f;
	}
}

void USpatialPingComponent::TickPingComponent()
{
	SendNewPing();
	// Pass latest measured ping to owning controller to be processed by PlayerState.
	OwningController->UpdatePing(RoundTripPing);
}

void USpatialPingComponent::SendNewPing()
{
	// Send a new ping using the current local time since start as both ID and timestamp.
	SendServerWorkerPingTimestamp(GetWorld()->GetRealTimeSeconds());
}

void USpatialPingComponent::OnRep_ReplicatedPingTimestamp()
{
	// If the new replicated ping timestamp is older than prev then we can ignore it.
	if (ReplicatedPingTimestamp < LastReceivedPingTimestamp)
	{
		return;
	}
	// Calculate the delta between the sent ping timestamp and the current time to determine round trip latency in seconds.
	RoundTripPing = GetWorld()->GetRealTimeSeconds() - ReplicatedPingTimestamp;
	// Update last received ping to match replicated.
	LastReceivedPingTimestamp = ReplicatedPingTimestamp;
}

bool USpatialPingComponent::SendServerWorkerPingTimestamp_Validate(float Timestamp)
{
	return true;
}

void USpatialPingComponent::SendServerWorkerPingTimestamp_Implementation(float Timestamp)
{
	ReplicatedPingTimestamp = Timestamp;
}
