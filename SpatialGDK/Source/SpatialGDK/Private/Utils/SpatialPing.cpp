// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialPing.h"
#include "Net/UnrealNetwork.h"

#include "Engine/Engine.h"

ASpatialPing::ASpatialPing(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	//TODO I imagine this needs to be considerably higher frequency
	NetUpdateFrequency = 1.f;

	SentPingIDs.Init(0, 0);
	SentPingTimes.Init(0.f, 0);
}

void ASpatialPing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialPing, ReplicatedPingID);
}

void ASpatialPing::BeginPlay()
{
	Super::BeginPlay();
}

void ASpatialPing::Destroyed()
{
	Super::Destroyed();
}

void ASpatialPing::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//TODO This needs a configurable toggle to enable/disable
	//TODO This needs some kind of rate limiting maybe? Do we want to ping every frame?
	SendNewPing();
}

void ASpatialPing::SendNewPing()
{
	uint32 NewPingID = LastSentPingID + 1;
	//TODO Needs some kind of overflow handling if this value ever ticks over
	SentPingIDs.Add(NewPingID);
	SentPingTimes.Add(GetWorld()->GetRealTimeSeconds());
	SendServerWorkerPingID(NewPingID);
	LastSentPingID = NewPingID;
}

void ASpatialPing::OnRep_ReplicatedPingID()
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
		return;
	}
	//If we get to here	then something went wrong as we received a valid ID that wasn't cached		
}

bool ASpatialPing::SendServerWorkerPingID_Validate(const uint32 PingID)
{
	return true;
}

void ASpatialPing::SendServerWorkerPingID_Implementation(const uint32 PingID)
{
	ReplicatedPingID = PingID;
}
