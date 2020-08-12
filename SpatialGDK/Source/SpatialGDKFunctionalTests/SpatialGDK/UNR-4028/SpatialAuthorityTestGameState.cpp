// Fill out your copyright notice in the Description page of Project Settings.


#include "SpatialAuthorityTestGameState.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASpatialAuthorityTestGameState::ASpatialAuthorityTestGameState()
	: Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ASpatialAuthorityTestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTestGameState, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestGameState, AuthorityOnTick);
}

// Called when the game starts or when spawned
void ASpatialAuthorityTestGameState::BeginPlay()
{
	Super::BeginPlay();
	
	//if( OwnerTest == nullptr )
	//{
	//	ensureMsgf(false, TEXT("AuthoritySpatialTestActor needs Test to be set"));
	//	return;
	//}

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
void ASpatialAuthorityTestGameState::Tick(float DeltaTime)
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

