// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RefreshActorDormancyTestActor.h"

#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/Material.h"
#include "Net/UnrealNetwork.h"

void ARefreshActorDormancyTestActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	if (PostTickFlushDelegateHandle.IsValid() && bIsSetDormancyComplete)
	{
		GetWorld()->OnPostTickFlush().Remove(PostTickFlushDelegateHandle);
		PostTickFlushDelegateHandle.Reset();
	}

	if (bSetupDormancyStateForTest && bFirstRep)
	{
		PostTickFlushDelegateHandle = GetWorld()->OnPostTickFlush().AddUObject(this, &ARefreshActorDormancyTestActor::FlushDormancy);
		bFirstRep = false;
	}

	Super::PreReplication(ChangedPropertyTracker);
}

void ARefreshActorDormancyTestActor::FlushDormancy()
{
	if (!bIsSetDormancyComplete)
	{
		SetNetDormancy(FinalDormancyState);
		bIsSetDormancyComplete = true;
	}
}

void ARefreshActorDormancyTestActor::SetInitiallyDormant(bool bInitiallyDormant)
{
	ENetDormancy OriginalDormancyState = bInitiallyDormant ? DORM_DormantAll : DORM_Awake;
	FinalDormancyState = bInitiallyDormant ? DORM_Awake : DORM_DormantAll;
	bSetupDormancyStateForTest = true;
	SetNetDormancy(OriginalDormancyState);
}
