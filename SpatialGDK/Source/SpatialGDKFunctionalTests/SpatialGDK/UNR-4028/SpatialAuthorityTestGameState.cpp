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

		AuthorityOnBeginPlay = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr
								   ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId()
								   : 1;
	}
}

void ASpatialAuthorityTestGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Needs to check HasActorBegunPlay() because the GameState Ticks before BeginPlay,
	// and at that point LoadBalanceStrategy is not initialized yet.
	if (HasAuthority() && AuthorityOnTick == 0 && HasActorBegunPlay())
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		AuthorityOnTick = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr
							  ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId()
							  : 1;
	}
}

