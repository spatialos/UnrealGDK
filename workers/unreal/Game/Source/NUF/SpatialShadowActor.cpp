// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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
	auto Map = CreateHandleToPropertyMap_Character();
	ReceiveUpdateFromSpatial_Character(PairedActor, Map, Update);
}
