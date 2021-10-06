// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlySpawnActorWithComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASpatialTestInitialOnlySpawnActorWithComponent::ASpatialTestInitialOnlySpawnActorWithComponent()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	InitialOnlyComponent = CreateDefaultSubobject<USpatialTestInitialOnlySpawnComponent>(FName("InitialOnlyComponent"));

	RootComponent = InitialOnlyComponent;
}

void ASpatialTestInitialOnlySpawnActorWithComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestInitialOnlySpawnActorWithComponent, InitialOnlyComponent);
}
