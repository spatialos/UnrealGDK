// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialSpawner.h"
#include "Commander.h"
#include "CoreMinimal.h"
#include "SpatialNetDriver.h"
#include "SpatialConstants.h"
#include "SpawnPlayerRequest.h"
#include "SpawnerServerComponent.h"
#include "SpawnerClientComponent.h"
#include "Engine/NetDriver.h"
#include "SpatialNetConnection.h"

ASpatialSpawner::ASpatialSpawner()
{
 	PrimaryActorTick.bCanEverTick = false;

	SpawnerClientComponent = CreateDefaultSubobject<USpawnerClientComponent>(TEXT("SpawnerClientComponent"));
	SpawnerServerComponent = CreateDefaultSubobject<USpawnerServerComponent>(TEXT("SpawnerServerComponent"));
}

void ASpatialSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("Initializing Spatial Spawner with netmode %d"), (int)GetNetMode());

	
	SpawnerServerComponent->OnAuthorityChange.AddDynamic(this, &ASpatialSpawner::HandleAuthorityChange);
	SpawnerClientComponent->OnAuthorityChange.AddDynamic(this, &ASpatialSpawner::HandleAuthorityChange);
		
	if (GetNetMode() == NM_Client)
	{
		// This logic is quite rudimentary, but it should work for now.
		// We are checking whether the worker has checked out the spawner yet (if so, it's ready to process our command).
		if (SpawnerServerComponent->IsComponentReady() && SpawnerServerComponent->Ready == true)
		{
			SendSpawnRequest();
		}
		else
		{
			SpawnerServerComponent->OnComponentUpdate.AddDynamic(this, &ASpatialSpawner::SendSpawnRequest);
		}		
	}
	else
	{
		SpawnerServerComponent->OnSpawnPlayerCommandRequest.AddDynamic(this, &ASpatialSpawner::HandleSpawnRequest);
	}

	if (SpawnerServerComponent->GetAuthority() == EAuthority::Authoritative)
	{
		SpawnerServerComponent->Ready = true;
	}

	if (SpawnerClientComponent->GetAuthority() == EAuthority::Authoritative)
	{
		SendSpawnRequest();
	}
}

void ASpatialSpawner::BeginPlay()
{
	Super::BeginPlay();	
}

void ASpatialSpawner::BeginDestroy()
{
	if (SpawnerServerComponent)
	{
		SpawnerServerComponent->OnSpawnPlayerCommandRequest.RemoveDynamic(this, &ASpatialSpawner::HandleSpawnRequest);
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
	auto response = NewObject<USpawnPlayerResponse>()->Init(improbable::spawner::SpawnPlayerResponse{});
	Responder->SendResponse(response);
}

void ASpatialSpawner::HandleAuthorityChange(EAuthority NewAuthority)
{
	if (SpawnerServerComponent->GetAuthority() == EAuthority::Authoritative)
	{
		SpawnerServerComponent->Ready = true;
	}

	if (SpawnerClientComponent->GetAuthority() == EAuthority::Authoritative)
	{
		SendSpawnRequest();
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
	if (SpawnerServerComponent->Ready == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Server hasn't checked out the spawner yet."));
		return;
	}

	USpawnPlayerRequest* NewRequest = NewObject<USpawnPlayerRequest>(this);
	NewRequest->SetUrl(GetNetDriver()->ServerConnection->URL.ToString());

	FSpawnPlayerCommandResultDelegate Callback;
	Callback.BindUFunction(this, "OnSpawnPlayerResponse");	
	
	SpawnerClientComponent->SendCommand()->SpawnPlayer(SpatialConstants::SPAWNER_ENTITY_ID, NewRequest, Callback, 0);
}
