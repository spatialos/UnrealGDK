// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestActor.h"

#include "SpatialComponentTestCallbackComponent.h"

ASpatialComponentTestActor::ASpatialComponentTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;

	CallbackComponent = CreateDefaultSubobject<USpatialComponentTestCallbackComponent>(FName("CallbackComponent"));

	RootComponent = CallbackComponent;
}
