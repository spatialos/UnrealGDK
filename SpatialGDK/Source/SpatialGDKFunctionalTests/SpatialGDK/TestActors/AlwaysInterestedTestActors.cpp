// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "AlwaysInterestedTestActors.h"

#include "Net/UnrealNetwork.h"

AAlwaysInterestedTestActor::AAlwaysInterestedTestActor()
{
	NetCullDistanceSquared = 1;
}

void AAlwaysInterestedTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InterestedActors);
}
