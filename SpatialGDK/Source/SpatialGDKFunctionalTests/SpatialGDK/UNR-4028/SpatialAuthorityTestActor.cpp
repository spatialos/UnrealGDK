// Fill out your copyright notice in the Description page of Project Settings.


#include "SpatialAuthorityTestActor.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

// Sets default values
ASpatialAuthorityTestActor::ASpatialAuthorityTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
}

void ASpatialAuthorityTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ASpatialAuthorityTestActor, OwnerTest);
	DOREPLIFETIME(ASpatialAuthorityTestActor, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestActor, AuthorityOnTick);
}

// Called when the game starts or when spawned
void ASpatialAuthorityTestActor::BeginPlay()
{
	Super::BeginPlay();
	
	//if( OwnerTest == nullptr )
	//{
	//	ensureMsgf(false, TEXT("AuthoritySpatialTestActor needs Test to be set"));
	//	return;
	//}

	if (HasAuthority())
	{
		AuthorityOnBeginPlay = Cast<USpatialNetDriver>(GetNetDriver())->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	}
}

// Called every frame
void ASpatialAuthorityTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AuthorityOnTick = Cast<USpatialNetDriver>(GetNetDriver())->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	}
}

