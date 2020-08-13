// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialAuthorityTestGameState.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"

ASpatialAuthorityTestGameState::ASpatialAuthorityTestGameState()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpatialAuthorityTestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTestGameState, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestGameState, AuthorityOnTick);
}

void ASpatialAuthorityTestGameState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && AuthorityOnBeginPlay == 0)
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr)
		{
			AuthorityOnBeginPlay = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
		}
	}
}

void ASpatialAuthorityTestGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && AuthorityOnTick == 0)
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr)
		{
			AuthorityOnTick = SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
		}
	}
}

