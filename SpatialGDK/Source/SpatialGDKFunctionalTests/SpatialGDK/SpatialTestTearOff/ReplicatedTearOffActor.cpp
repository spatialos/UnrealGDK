// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTearOffActor.h"
#include "Net/UnrealNetwork.h"

AReplicatedTearOffActor::AReplicatedTearOffActor()
{
	bNetLoadOnClient = true;
	SetReplicatingMovement(true);
	TestInteger = 0;
	bFirstTick = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AReplicatedTearOffActor::BeginPlay()
{
	Super::BeginPlay();

	bFirstTick = true;
}

void AReplicatedTearOffActor::Tick(float DeltaSeconds)
{
	if (bFirstTick)
	{
		// Note:  calling TearOff inside the constructor will not prevent the Actor from replicating.
		// Note2: calling TearOff inside BeginPlay will produce an error if using the ReplicationGraph.
		TearOff();
		bFirstTick = false;
	}
}

void AReplicatedTearOffActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTearOffActor, TestInteger);
}
