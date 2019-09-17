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

	SentPingIDs.Init(0, 0);
	SentPingTimes.Init(0.f, 0);
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
		//Only run on clients, will not run in single player / standalone
		if (GEngine->GetNetMode(World) == NM_Client)
		{
			World->GetTimerManager().SetTimer(PingTimerHandle, this, &USpatialPingComponent::TickPingComponent, PingFrequency, true);
		}
	}
}

void USpatialPingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

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
	uint32 NewPingID = LastSentPingID + 1;
	//TODO Needs some kind of overflow handling if this value ever ticks over?
	SentPingIDs.Add(NewPingID);
	SentPingTimes.Add(GetWorld()->GetRealTimeSeconds());
	SendServerWorkerPingID(NewPingID);
	LastSentPingID = NewPingID;
}

void USpatialPingComponent::OnRep_ReplicatedPingID()
{
	//If the new replicated ping ID is lower than prev then we can ignore it
	if (ReplicatedPingID < LastReceivedPingID)
	{
		return;
	}
	int32 PingIndex;
	//Find replicated ping in map
	if (SentPingIDs.Find(ReplicatedPingID, PingIndex))
	{
		//Calculate time delta and update ping value somewhere
		RTPing = GetWorld()->GetRealTimeSeconds() - SentPingTimes[PingIndex];
		//Remove matching element from the map and ALL preceding elements
		SentPingIDs.RemoveAt(0, PingIndex + 1);
		SentPingTimes.RemoveAt(0, PingIndex + 1);
		//Update last received ping to match replicated
		LastReceivedPingID = ReplicatedPingID;
		UE_LOG(LogTemp, Warning, TEXT("RT Ping = %f"), RTPing);
		return;
	}
	//If we get to here	then something went wrong as we received a valid ID that wasn't cached		
}

bool USpatialPingComponent::SendServerWorkerPingID_Validate(const uint32 PingID)
{
	return true;
}

void USpatialPingComponent::SendServerWorkerPingID_Implementation(const uint32 PingID)
{
	ReplicatedPingID = PingID;
}
