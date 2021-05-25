// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicSubObjectTestActor.h"
#include "Net/UnrealNetwork.h"

ADynamicSubObjectTestActor::ADynamicSubObjectTestActor()
{
	TestIntProperty = -1;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
	bReplicates = true;
}

void ADynamicSubObjectTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADynamicSubObjectTestActor, TestIntProperty);
}
