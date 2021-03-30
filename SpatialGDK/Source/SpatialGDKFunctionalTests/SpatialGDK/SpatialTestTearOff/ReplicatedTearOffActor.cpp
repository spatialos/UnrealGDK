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
		//        (when spawning a dynamic actor, the BeginPlay is called immediately upon spawn, so we are still in the actor creation
		//        flow. It turns out actors are added to replication lists just after beginPlay is called on them (still within the same
		//        callstack, but just a bit too late). So tearing off an actor with replication graph will attempt to remove it from the
		//        replication list, but can't find it there, producing an error. Startup actors are okay, because their BeginPlay is
		//        delayed.)
		TearOff();
		bFirstTick = false;
	}
}

void AReplicatedTearOffActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTearOffActor, TestInteger);
}
