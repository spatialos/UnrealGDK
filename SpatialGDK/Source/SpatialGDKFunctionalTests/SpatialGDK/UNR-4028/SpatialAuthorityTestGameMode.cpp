// Fill out your copyright notice in the Description page of Project Settings.


#include "SpatialAuthorityTestGameMode.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialAuthorityTestGameState.h"

// Sets default values
ASpatialAuthorityTestGameMode::ASpatialAuthorityTestGameMode()
	: Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ASpatialAuthorityTestGameState::StaticClass();
}

void ASpatialAuthorityTestGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTestGameMode, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestGameMode, AuthorityOnTick);
}

// Called when the game starts or when spawned
void ASpatialAuthorityTestGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr)
		{
			AuthorityOnBeginPlay = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
		}
	}
}

// Called every frame
void ASpatialAuthorityTestGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr)
		{
			AuthorityOnTick = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
		}
	}
}

