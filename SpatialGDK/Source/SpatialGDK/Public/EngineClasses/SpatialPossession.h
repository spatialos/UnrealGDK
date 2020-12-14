// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPossession, Log, All);

#include "SpatialPossession.generated.h"

UCLASS()
class SPATIALGDK_API USpatialPossession : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Possession")
	static void RemotePossess(AController* Controller, APawn* Pawn);

	static void PossessAfterMigration(AController& Controller);
};
