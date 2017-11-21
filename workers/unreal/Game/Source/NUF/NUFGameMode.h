// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SpatialOS/Generated/UClasses/EntityTemplate.h"
#include "GameFramework/GameModeBase.h"
#include "NUFGameMode.generated.h"

UCLASS(minimalapi)
class ANUFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANUFGameMode();

	//UFUNCTION(BlueprintPure, Category = "NUFGameMode")
	//UEntityTemplate* CreatePlayerEntityTemplate(FString clientWorkerId, const FVector& position);
};



