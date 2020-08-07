// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestWorldComposition.h"

#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"

#include "Kismet/GameplayStatics.h"

ASpatialTestWorldComposition::ASpatialTestWorldComposition()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test World Composition");
}

void ASpatialTestWorldComposition::BeginPlay()
{
	Super::BeginPlay();
}

