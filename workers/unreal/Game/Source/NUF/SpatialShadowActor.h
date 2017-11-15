// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "UnrealACharacterReplicatedDataComponent.h"
#include "UnrealACharacterCompleteDataComponent.h"
#include "SpatialShadowActor.generated.h"

UCLASS()
class ASpatialShadowActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialShadowActor();

	/** Replication component */
	UPROPERTY()
	UUnrealACharacterReplicatedDataComponent* ReplicatedData;

	/** Complete data component */
	UPROPERTY()
	UUnrealACharacterCompleteDataComponent* CompleteData;
};

