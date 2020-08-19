// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTestActorComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

USpatialAuthorityTestActorComponent::USpatialAuthorityTestActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	SetIsReplicatedByDefault(true);
}

void USpatialAuthorityTestActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialAuthorityTestActorComponent, ReplicatedAuthorityOnBeginPlay);
}

void USpatialAuthorityTestActorComponent::OnAuthorityGained()
{
	NumAuthorityGains += 1;
}

void USpatialAuthorityTestActorComponent::OnAuthorityLost()
{
	NumAuthorityLosses += 1;
}

void USpatialAuthorityTestActorComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	if (Owner->HasAuthority() && AuthorityOnBeginPlay == 0)
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Owner->GetNetDriver());

		AuthorityOnBeginPlay = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId() : 1;

		ReplicatedAuthorityOnBeginPlay = AuthorityOnBeginPlay;
	}
}

void USpatialAuthorityTestActorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
														FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();

	// Need to check HasActorBegunPlay because GameState will tick before BeginPlay.
	if (Owner->HasActorBegunPlay() && Owner->HasAuthority() && AuthorityOnTick == 0)
	{
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(Owner->GetNetDriver());
		AuthorityOnTick = SpatialNetDriver != nullptr && SpatialNetDriver->LoadBalanceStrategy != nullptr
							  ? SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId()
							  : 1;
	}
}
