// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlySpawnActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASpatialTestInitialOnlySpawnActor::ASpatialTestInitialOnlySpawnActor()
{
	bReplicates = true;
	NetCullDistanceSquared = 10000.0f;
}

void ASpatialTestInitialOnlySpawnActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASpatialTestInitialOnlySpawnActor, Int_Initial, COND_InitialOnly);
	DOREPLIFETIME(ASpatialTestInitialOnlySpawnActor, Int_Replicate);
}
