// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialSpawner.h"
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
	UE_LOG(LogTemp, Warning, TEXT("Spawn Request Received!"));
}

