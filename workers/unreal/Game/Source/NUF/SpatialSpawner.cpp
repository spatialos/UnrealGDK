// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialSpawner.h"
#include "Commander.h"
#include "CoreMinimal.h"
#include "SpatialNetDriver.h"
#include "SpatialConstants.h"
#include "SpawnPlayerRequest.h"
#include "SpawnerComponent.h"
#include "Engine/NetDriver.h"
#include "SpatialNetConnection.h"

ASpatialSpawner::ASpatialSpawner()
{
 	PrimaryActorTick.bCanEverTick = false;

	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));
}

void ASpatialSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("Initializing Spatial Spawner with netmode %d"), (int)GetNetMode());

	SpawnerComponent->OnSpawnPlayerCommandRequest.AddDynamic(this, &ASpatialSpawner::HandleSpawnRequest);

	SpawnerComponent->OnAuthorityChange.AddDynamic(this, &ASpatialSpawner::HandleAuthorityChange);
		
	if (GetNetMode() == NM_Client)
	{
		if (SpawnerComponent->IsComponentReady())
		{
			SendSpawnRequest();
		}
		else
		{
			SpawnerComponent->OnComponentReady.AddDynamic(this, &ASpatialSpawner::SendSpawnRequest);
		}		
	}

	if (SpawnerComponent->GetAuthority() == EAuthority::Authoritative)
	{
		SpawnerComponent->Ready = true;
	}
}

void ASpatialSpawner::BeginPlay()
{
	Super::BeginPlay();	
}

void ASpatialSpawner::BeginDestroy()
{
	if (SpawnerComponent)
	{
		SpawnerComponent->OnSpawnPlayerCommandRequest.RemoveDynamic(this, &ASpatialSpawner::HandleSpawnRequest);
	}	
	
	Super::BeginDestroy();	
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

void ASpatialSpawner::HandleAuthorityChange(EAuthority NewAuthority)
{
	if (NewAuthority == EAuthority::Authoritative)
	{
		SpawnerComponent->Ready = true;
	}
}

void ASpatialSpawner::OnSpawnPlayerResponse(const FSpatialOSCommandResult& result, USpawnPlayerResponse* response)
{
	if (!result.Success())
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn Player failed."));
	}	
}

void ASpatialSpawner::SendSpawnRequest()
{
	USpawnPlayerRequest* NewRequest = NewObject<USpawnPlayerRequest>(this);
	NewRequest->SetUrl(GetNetDriver()->ServerConnection->URL.ToString());

	FSpawnPlayerCommandResultDelegate Callback;
	Callback.BindUFunction(this, "OnSpawnPlayerResponse");	
	
	SpawnerComponent->SendCommand()->SpawnPlayer(SpatialConstants::SPAWNER_ENTITY_ID, NewRequest, Callback, 0);
}

