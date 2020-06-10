// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "OwnerOnlyTestPawn.h"
#include "SpatialFunctionalTest.h"
#include "OwnerOnlyPropertyReplication.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AOwnerOnlyPropertyReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AOwnerOnlyPropertyReplication();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	AOwnerOnlyTestPawn* Pawn = nullptr;

	float StepTimer = 0.0f;

	TArray<TPair<AController*, APawn*>> OriginalPawns;
};
