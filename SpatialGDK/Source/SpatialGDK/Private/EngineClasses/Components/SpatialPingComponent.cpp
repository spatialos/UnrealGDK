// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/SpatialPingComponent.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

USpatialPingComponent::USpatialPingComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

	UWorld* World = GetWorld();
	if (World)
	{
		//Only run on clients
		if (GEngine->GetNetMode(World) == NM_Client)
		{
			World->GetTimerManager().SetTimer(PingTimerHandle, this, &USpatialPingComponent::TickPingComponent, PingFrequency, true);
		}
	}
}

void USpatialPingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//Ensure the timer is cleaned up
	GetWorld()->GetTimerManager().ClearTimer(PingTimerHandle);
}

float USpatialPingComponent::GetPing() const
{
	return RTPing;
}

void USpatialPingComponent::TickPingComponent()
{
	SendNewPing();
	//If component is attached to a player controller then forward the ping on to the controller's PlayerState
	APlayerController* Controller = Cast<APlayerController>(GetOwner());
	if (Controller)
	{
		Controller->UpdatePing(RTPing);
	}
}

void USpatialPingComponent::SendNewPing()
{
	//Send a new ping using the current local time as both ID and timestamp
	SendServerWorkerPingID(GetWorld()->GetRealTimeSeconds());
}

void USpatialPingComponent::OnRep_ReplicatedPingID()
{
	//If the new replicated ping timestamp is older than prev then we can ignore it
	if (ReplicatedPingID < LastReceivedPingID)
	{
		return;
	}
	//Calculate the delta between the sent ping timestamp and the current time to determine round trip latency
	RTPing = GetWorld()->GetRealTimeSeconds() - ReplicatedPingID;
	//Update last received ping to match replicated
	LastReceivedPingID = ReplicatedPingID;
}

bool USpatialPingComponent::SendServerWorkerPingID_Validate(const float PingID)
{
	return true;
}

void USpatialPingComponent::SendServerWorkerPingID_Implementation(const float PingID)
{
	ReplicatedPingID = PingID;
}
