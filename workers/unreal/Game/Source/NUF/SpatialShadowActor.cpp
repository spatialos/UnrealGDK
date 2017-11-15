// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpatialShadowActor.h"

ASpatialShadowActor::ASpatialShadowActor()
{
	SetActorTickEnabled(false);

	ReplicatedData = CreateDefaultSubobject<UUnrealACharacterReplicatedDataComponent>(TEXT("UnrealACharacterReplicatedDataComponent"));
	CompleteData = CreateDefaultSubobject<UUnrealACharacterCompleteDataComponent>(TEXT("UnrealACharacterCompleteDataComponent"));
}
