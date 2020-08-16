// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTearOffActor.h"
#include "Net/UnrealNetwork.h"

AReplicatedTearOffActor::AReplicatedTearOffActor()
{
	bNetLoadOnClient = true;
	bReplicateMovement = true;
	TestInteger = 0;
}

void AReplicatedTearOffActor::BeginPlay()
{
	Super::BeginPlay();

	// Note:  calling TearOff inside the constructor will not prevent the Actor from replicating.
	TearOff();
}

void AReplicatedTearOffActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTearOffActor, TestInteger);
}
