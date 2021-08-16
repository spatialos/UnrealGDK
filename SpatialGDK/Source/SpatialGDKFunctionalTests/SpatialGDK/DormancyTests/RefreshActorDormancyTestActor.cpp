// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RefreshActorDormancyTestActor.h"

#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/Material.h"
#include "Net/UnrealNetwork.h"

ARefreshActorDormancyTestActor::ARefreshActorDormancyTestActor()
{
	NetDormancy = DORM_Awake;
}
