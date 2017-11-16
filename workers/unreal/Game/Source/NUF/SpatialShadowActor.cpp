// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpatialShadowActor.h"
#include "Generated/SpatialInteropCharacter.h"

ASpatialShadowActor::ASpatialShadowActor() : PairedActor(nullptr)
{
	SetActorTickEnabled(false);

	ReplicatedData = CreateDefaultSubobject<UUnrealACharacterReplicatedDataComponent>(TEXT("UnrealACharacterReplicatedDataComponent"));
	CompleteData = CreateDefaultSubobject<UUnrealACharacterCompleteDataComponent>(TEXT("UnrealACharacterCompleteDataComponent"));
}

void ASpatialShadowActor::OnReplicatedDataUpdate(UUnrealACharacterReplicatedDataComponentUpdate* Update)
{
	auto Map = CreateCmdIndexToPropertyMap_Character();
	ReceiveUpdateFromSpatial_Character(PairedActor, Map, Update);
}
