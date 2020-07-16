// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRepossession.generated.h"

class ATestPossessionPawn;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRepossession : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestRepossession();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<APlayerController*> Controllers;
	UPROPERTY(Replicated)
	TArray<ATestPossessionPawn*> TestPawns;

	// To possess original pawns
	TArray<TPair<AController*, APawn*>> OriginalPawns;
};
