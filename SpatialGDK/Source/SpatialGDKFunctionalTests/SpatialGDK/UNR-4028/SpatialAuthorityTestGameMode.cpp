// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialAuthorityTestGameMode.h"
#include "Components/SceneComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialAuthorityTestGameState.h"

ASpatialAuthorityTestGameMode::ASpatialAuthorityTestGameMode()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = ASpatialAuthorityTestGameState::StaticClass();
}

void ASpatialAuthorityTestGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialAuthorityTestGameMode, AuthorityOnBeginPlay);
	DOREPLIFETIME(ASpatialAuthorityTestGameMode, AuthorityOnTick);
}

void ASpatialAuthorityTestGameMode::BeginPlay()
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

void ASpatialAuthorityTestGameMode::Tick(float DeltaTime)
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

