// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialShadowActor.h"

ASpatialShadowActor::ASpatialShadowActor() : PairedActor(nullptr)
{
	SetActorTickEnabled(false);

	//ReplicatedData->OnComponentUpdate.AddDynamic(this, &ASpatialShadowActor::OnReplicatedDataUpdate);
}

void ASpatialShadowActor::ReplicateChanges(float DeltaTime)
{
}
