// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTestActor.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

ASpatialAuthorityTestActor::ASpatialAuthorityTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
}

void ASpatialAuthorityTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTestActor, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestActor, AuthorityOnTick);
}

void ASpatialAuthorityTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && AuthorityOnBeginPlay == 0)
	{
		AuthorityOnBeginPlay = Cast<USpatialNetDriver>(GetNetDriver())->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	}
}

void ASpatialAuthorityTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && AuthorityOnTick == 0)
	{
		AuthorityOnTick = Cast<USpatialNetDriver>(GetNetDriver())->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	}
}

