// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DormancyTestActor.h"

#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/Material.h"
#include "Net/UnrealNetwork.h"

ADormancyTestActor::ADormancyTestActor()
{
	TestIntProp = 0;

	NetDormancy = DORM_Initial; // By default dormant initially, as we have no way to correctly set this at runtime.
}

void ADormancyTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADormancyTestActor, TestIntProp);
}
