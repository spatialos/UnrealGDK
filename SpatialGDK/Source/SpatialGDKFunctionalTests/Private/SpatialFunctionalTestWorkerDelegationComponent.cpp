// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestWorkerDelegationComponent.h"
#include "Net/UnrealNetwork.h"

USpatialFunctionalTestWorkerDelegationComponent::USpatialFunctionalTestWorkerDelegationComponent()
	: Super()
{
#if ENGINE_MINOR_VERSION <= 23
	bReplicates = true;
#else
	SetIsReplicatedByDefault(true);
#endif
}

void USpatialFunctionalTestWorkerDelegationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialFunctionalTestWorkerDelegationComponent, WorkerId);
	DOREPLIFETIME(USpatialFunctionalTestWorkerDelegationComponent, bIsPersistent);
}
