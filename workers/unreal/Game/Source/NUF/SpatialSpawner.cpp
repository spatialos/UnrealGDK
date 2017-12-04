// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialSpawner.h"
#include "CoreMinimal.h"
#include "SpatialNetDriver.h"
#include "SpawnPlayerRequest.h"
#include "SpawnerComponent.h"

ASpatialSpawner::ASpatialSpawner()
{
 	PrimaryActorTick.bCanEverTick = false;

	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));
}

void ASpatialSpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnerComponent->OnSpawnPlayerCommandRequest.AddDynamic(this, &ASpatialSpawner::HandleSpawnRequest);
}

void ASpatialSpawner::BeginDestroy()
{
	Super::BeginDestroy();

	SpawnerComponent->OnSpawnPlayerCommandRequest.RemoveDynamic(this, &ASpatialSpawner::HandleSpawnRequest);
}

void ASpatialSpawner::HandleSpawnRequest(USpawnPlayerCommandResponder * Responder)
{
	check(GetWorld());	

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

	if (NetDriver)
	{
		NetDriver->AcceptNewPlayer(FURL(nullptr, *(Responder->GetRequest()->GetUrl()), TRAVEL_Absolute));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Login failed. Spatial net driver is not setup correctly."));
	}
}

