// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RefreshActorDormancyTestActor.h"

#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/Material.h"
#include "Net/UnrealNetwork.h"

ARefreshActorDormancyTestActor::ARefreshActorDormancyTestActor()
{
	NetDormancy = DORM_Initial;
}

bool ARefreshActorDormancyTestActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	if (bSetupForTest && bIsSetDormancyComplete)
	{
		GetWorld()->OnPostTickFlush().Remove(PostTickFlushDelegateHandle);
		bIsSetDormancyComplete = false;
	}

	if (bSetupForTest && bfirstRep)
	{
		PostTickFlushDelegateHandle = GetWorld()->OnPostTickFlush().AddUObject(this, &ARefreshActorDormancyTestActor::FlushDormancy);
		bfirstRep = false;
	}

	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void ARefreshActorDormancyTestActor::FlushDormancy()
{
	if (!bIsSetDormancyComplete)
	{
		SetNetDormancy(FinalDormancyState);
		bIsSetDormancyComplete = true;
	}
}

void ARefreshActorDormancyTestActor::SetupForTest(bool bGoToAwakeState)
{
	ENetDormancy OriginalDormancyState = bGoToAwakeState ? DORM_DormantAll : DORM_Awake;
	FinalDormancyState = bGoToAwakeState ? DORM_Awake : DORM_DormantAll;
	bSetupForTest = true;
	SetNetDormancy(OriginalDormancyState);
}
