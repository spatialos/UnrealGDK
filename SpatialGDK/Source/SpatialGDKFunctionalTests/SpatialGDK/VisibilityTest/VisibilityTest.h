// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "VisibilityTest.generated.h"

class ATestMovementCharacter;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AVisibilityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	AVisibilityTest();

	virtual void BeginPlay() override;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<APlayerController*> Controllers;
	UPROPERTY(Replicated)
	TArray<ATestMovementCharacter*> TestPawns;

	// To possess original pawns
	TArray<TPair<AController*, APawn*>> OriginalPawns;

	FVector CharacterRemoteLocation;
	FVector Character1StartingLocation;
	FVector Character2StartingLocation;
};
