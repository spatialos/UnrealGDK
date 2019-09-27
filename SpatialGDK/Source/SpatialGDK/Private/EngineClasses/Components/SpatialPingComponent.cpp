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

	DOREPLIFETIME(USpatialPingComponent, ReplicatedPingID);
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
		LastSentPingID = 0;
		// (Re)initialize TArrays
		SentPingIDs.Init(0, 0);
		SentPingTimestamps.Init((double)0, 0);
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
	// Generate a new ping ID
	int16 NewPingID = LastSentPingID + 1;
	SentPingIDs.Add(NewPingID);
	SentPingTimestamps.Add(FPlatformTime::Seconds());
	SendServerWorkerPingID(NewPingID);
	LastSentPingID = NewPingID;
}

void USpatialPingComponent::OnRep_ReplicatedPingID()
{
	int32 PingIndex;
	//Find replicated ping in ID array
	if (SentPingIDs.Find(ReplicatedPingID, PingIndex))
	{
		//Calculate time delta and update ping value somewhere
		RoundTripPing = (float)(FPlatformTime::Seconds() - SentPingTimestamps[PingIndex]);
		//Remove matching elements from the arrays and ALL preceding elements
		SentPingIDs.RemoveAt(0, PingIndex + 1);
		SentPingTimestamps.RemoveAt(0, PingIndex + 1);
		return;
	}
}

bool USpatialPingComponent::SendServerWorkerPingID_Validate(int16 PingID)
{
	return true;
}

void USpatialPingComponent::SendServerWorkerPingID_Implementation(int16 PingID)
{
	ReplicatedPingID = PingID;
}
