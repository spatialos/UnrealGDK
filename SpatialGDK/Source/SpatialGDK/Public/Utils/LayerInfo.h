// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LayerInfo.generated.h"

USTRUCT()
struct FLayerInfo
{
	GENERATED_BODY()

	FLayerInfo() : Name(NAME_None)
	{
	}

	UPROPERTY()
	FName Name;

	// Using TSoftClassPtr here to prevent eagerly loading all classes.
	/** The Actor classes contained within this group. Children of these classes will also be included. */
	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	TSet<TSoftClassPtr<AActor>> ActorClasses;
}; 
