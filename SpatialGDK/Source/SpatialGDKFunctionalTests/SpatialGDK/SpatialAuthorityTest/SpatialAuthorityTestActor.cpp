// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthorityTestActor.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialAuthorityTestActorComponent.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

ASpatialAuthorityTestActor::ASpatialAuthorityTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	AuthorityComponent = CreateDefaultSubobject<USpatialAuthorityTestActorComponent>(FName("AuthorityComponent"));

	RootComponent = AuthorityComponent;
}
