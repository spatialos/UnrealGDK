// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestActor.h"
#include "Net/UnrealNetwork.h"

ASpatialSnapshotTestActor::ASpatialSnapshotTestActor()
	: Super()
{
}

void ASpatialSnapshotTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASpatialSnapshotTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
