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
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());

		AuthorityOnBeginPlay = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId() : 1;
	}
}

void ASpatialAuthorityTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && AuthorityOnTick == 0)
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		AuthorityOnTick = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId() : 1;
	}
}

