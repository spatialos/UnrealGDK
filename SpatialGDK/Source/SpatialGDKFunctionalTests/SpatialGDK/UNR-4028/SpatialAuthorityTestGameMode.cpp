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
		AuthorityOnBeginPlay = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr
								   ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId()
								   : 1;
	}
}

void ASpatialAuthorityTestGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Needs to check HasActorBegunPlay() because the GameMode Ticks before BeginPlay,
	// and at that point LoadBalanceStrategy is not initialized yet.
	if (HasAuthority() && AuthorityOnTick == 0 && HasActorBegunPlay())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		AuthorityOnTick = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr
							  ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId()
							  : 1;
	}
}

