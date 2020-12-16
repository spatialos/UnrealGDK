// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestRemotePossession.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestRemotePossession : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestRemotePossession();

	virtual void PrepareTest() override;

	static void HandlePossessionFailed(ERemotePossessFailure Failure);

	// To save original Pawns and possess them back at the end
	UPROPERTY(replicated)
	TArray<APawn*> OriginalPawns;
};
