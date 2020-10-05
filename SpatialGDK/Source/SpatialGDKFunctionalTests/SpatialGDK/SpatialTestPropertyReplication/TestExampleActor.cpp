// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestExampleActor.h"
#include "Net/UnrealNetwork.h"

ATestExampleActor::ATestExampleActor()
{
	ExampleReplicatedProperty = 0;
}

void ATestExampleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestExampleActor, ExampleReplicatedProperty);
}
