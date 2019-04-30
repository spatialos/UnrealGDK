// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGameState.h"
#include "Net/UnrealNetwork.h"
#include "SpatialGDKSettings.h"
#include "SpatialMetricsDisplay.h"

ASpatialGameState::ASpatialGameState() :
	SpatialMetricsDisplay(nullptr)
{
	SpatialMetricsDisplayClass = ASpatialMetricsDisplay::StaticClass();
}

void ASpatialGameState::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	if (Role == ROLE_Authority && GetDefault<USpatialGDKSettings>()->bReplicateServerStatsToClients)
	{
		SpawnSpatialMetricsDisplay();
	}
#endif
}
void ASpatialGameState::SpawnSpatialMetricsDisplay()
{
	check(Role == ROLE_Authority);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FVector SpawnLocation(0, 0, 0);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	SpatialMetricsDisplay = Cast<ASpatialMetricsDisplay>(GetWorld()->SpawnActor(SpatialMetricsDisplayClass, &SpawnLocation, &SpawnRotation, SpawnParams));
}

void ASpatialGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialGameState, SpatialMetricsDisplay);
}

void ASpatialGameState::SpatialToggleStatDisplay()
{
#if !UE_BUILD_SHIPPING
	check(Role != ROLE_Authority);

	if (SpatialMetricsDisplay != nullptr)
	{
		SpatialMetricsDisplay->ToggleStatDisplay();
	}
#endif
}
