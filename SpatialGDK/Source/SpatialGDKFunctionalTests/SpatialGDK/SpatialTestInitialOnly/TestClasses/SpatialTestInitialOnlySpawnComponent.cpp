// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlySpawnComponent.h"
#include "Net/UnrealNetwork.h"

USpatialTestInitialOnlySpawnComponent::USpatialTestInitialOnlySpawnComponent()
{
	SetIsReplicatedByDefault(true);
}

void USpatialTestInitialOnlySpawnComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USpatialTestInitialOnlySpawnComponent, Int_Initial, COND_InitialOnly);
	DOREPLIFETIME(USpatialTestInitialOnlySpawnComponent, Int_Replicate);
}
