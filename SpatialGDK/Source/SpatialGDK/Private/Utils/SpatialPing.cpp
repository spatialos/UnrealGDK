// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialPing.h"

#include "Engine/Engine.h"

ASpatialPing::ASpatialPing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 1.f;
}

void ASpatialPing::BeginPlay()
{
	Super::BeginPlay();
}

void ASpatialPing::Destroyed()
{
	Super::Destroyed();
}

void ASpatialPing::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
