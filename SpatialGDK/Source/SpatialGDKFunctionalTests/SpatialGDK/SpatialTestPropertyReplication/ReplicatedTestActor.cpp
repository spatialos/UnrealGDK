// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActor.h"
#include "Net/UnrealNetwork.h"

AReplicatedTestActor::AReplicatedTestActor()
{
	ExampleReplicatedProperty = 0;
}

void AReplicatedTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTestActor, ExampleReplicatedProperty);
}
