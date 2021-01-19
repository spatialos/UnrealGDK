// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestHandover/HandoverCube.h"
#include "Utils/SpatialStatics.h"
#include "DynamicReplicationHandoverCube.generated.h"

/**
 * A replicated Actor with a Cube Mesh, used as a base for Actors in spatial tests.
 */
UCLASS()
class ADynamicReplicationHandoverCube : public AHandoverCube
{
	GENERATED_BODY()

public:
	ADynamicReplicationHandoverCube();

	static constexpr int BasicTestPropertyValue = 10;
	static constexpr int UpdatedTestPropertyValue = 100;

	UPROPERTY(Handover)
	int HandoverTestProperty = BasicTestPropertyValue;
};
