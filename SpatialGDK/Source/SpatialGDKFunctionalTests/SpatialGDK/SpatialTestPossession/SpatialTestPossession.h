// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPossession.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPossession : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestPossession();

	virtual void BeginPlay() override;

	// To save original Pawns and possess them back at the end
	TArray<TPair<AController*, APawn*>> OriginalPawns;
};
