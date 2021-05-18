// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedGASTestActor.h"
#include "Net/UnrealNetwork.h"

AReplicatedGASTestActor::AReplicatedGASTestActor()
{
	TestIntProperty = -1;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
	bReplicates = true;
}

void AReplicatedGASTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedGASTestActor, TestIntProperty);
}
