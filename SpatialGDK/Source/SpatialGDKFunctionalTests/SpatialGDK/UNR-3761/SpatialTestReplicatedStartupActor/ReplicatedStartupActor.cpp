// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedStartupActor.h"
#include "Net/UnrealNetwork.h"

AReplicatedStartupActor::AReplicatedStartupActor()
{
	SetReplicateMovement(true);
}

void AReplicatedStartupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedStartupActor, TestIntProperty);
	DOREPLIFETIME(AReplicatedStartupActor, TestArrayProperty);
	DOREPLIFETIME(AReplicatedStartupActor, TestArrayStructProperty);
}
