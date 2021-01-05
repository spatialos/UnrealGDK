// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestActor.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialComponentTestActorComponent.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

ASpatialComponentTestActor::ASpatialComponentTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	CallbackComponent = CreateDefaultSubobject<USpatialComponentTestActorComponent>(FName("CallbackComponent"));

	RootComponent = CallbackComponent;
}
