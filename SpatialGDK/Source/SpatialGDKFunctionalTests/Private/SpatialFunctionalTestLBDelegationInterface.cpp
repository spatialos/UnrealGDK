// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestLBDelegationInterface.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTestWorkerDelegationComponent.h"
#include "Utils/SpatialStatics.h"

bool ISpatialFunctionalTestLBDelegationInterface::AddActorDelegation(AActor* Actor, VirtualWorkerId WorkerId,
																	 bool bPersistOnTestFinished /*= false*/)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (!Actor->HasAuthority())
	{
		ensureMsgf(false, TEXT("Only the worker authoritative over an Actor can delegate it to another worker. Tried to delegate %s to %d"),
				   *Actor->GetName(), WorkerId);
		return false;
	}

	USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent = Cast<USpatialFunctionalTestWorkerDelegationComponent>(
		Actor->GetComponentByClass(USpatialFunctionalTestWorkerDelegationComponent::StaticClass()));

	if (DelegationComponent == nullptr)
	{
		DelegationComponent = NewObject<USpatialFunctionalTestWorkerDelegationComponent>(Actor, "Delegation Component");
		DelegationComponent->RegisterComponent();
	}

	DelegationComponent->WorkerId = WorkerId;
	DelegationComponent->bIsPersistent = bPersistOnTestFinished;

	return true;
}

bool ISpatialFunctionalTestLBDelegationInterface::RemoveActorDelegation(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent =
		Actor->FindComponentByClass<USpatialFunctionalTestWorkerDelegationComponent>();
	if (DelegationComponent == nullptr)
	{
		return false;
	}

	DelegationComponent->DestroyComponent();

	return true;
}

bool ISpatialFunctionalTestLBDelegationInterface::HasActorDelegation(AActor* Actor, VirtualWorkerId& WorkerId, bool& bIsPersistent)
{
	WorkerId = 0;
	bIsPersistent = false;

	if (Actor == nullptr)
	{
		return false;
	}
	USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent =
		Actor->FindComponentByClass<USpatialFunctionalTestWorkerDelegationComponent>();

	if (DelegationComponent != nullptr)
	{
		WorkerId = DelegationComponent->WorkerId;
		bIsPersistent = DelegationComponent->bIsPersistent;
		return true;
	}

	return false;
}

void ISpatialFunctionalTestLBDelegationInterface::RemoveAllActorDelegations(UWorld* World, bool bRemovePersistent /*= false*/)
{
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->HasAuthority())
		{
			USpatialFunctionalTestWorkerDelegationComponent* DelegationComponent =
				It->FindComponentByClass<USpatialFunctionalTestWorkerDelegationComponent>();
			if (DelegationComponent != nullptr)
			{
				if (!DelegationComponent->bIsPersistent || bRemovePersistent)
				{
					DelegationComponent->DestroyComponent();
				}
			}
		}
	}
}
