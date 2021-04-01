// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Test1x2GridStrategy.h"
#include "SpatialTest1x2GridSmallInterestStrategy.generated.h"

/**
 * A 1 by 2 (rows by columns) load balancing strategy for testing zoning features.
 * Has a 150 unit interest border, a very small border to create a narrow range in which an actor is visible to both server-workers.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTest1x2GridSmallInterestStrategy : public UTest1x2GridStrategy
{
	GENERATED_BODY()

public:
	USpatialTest1x2GridSmallInterestStrategy();
};
