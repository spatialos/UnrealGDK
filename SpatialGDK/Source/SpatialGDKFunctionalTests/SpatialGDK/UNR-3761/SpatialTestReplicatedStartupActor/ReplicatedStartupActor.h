// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedStartupActor.generated.h"

USTRUCT()
struct FTestStruct
{
	GENERATED_BODY()

	UPROPERTY()
	int Int;
};

/**
 * Helper actor used in SpatialTestReplicatedStartupActor.
 * Contains 3 replicated properties: an int, an array of ints, and an array of FTestStruct.
 */

UCLASS()
class AReplicatedStartupActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedStartupActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestIntProperty;

	UPROPERTY(Replicated)
	TArray<int> TestArrayProperty;

	UPROPERTY(Replicated)
	TArray<FTestStruct> TestArrayStructProperty;
};
