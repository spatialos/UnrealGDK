// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Test2x2GridStrategy.generated.h"

/**
 * A 2 by 2 (rows by columns) load balancing strategy for testing zoning features.
 * Has a world-wide interest border, so everything should be in view.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest2x2GridStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest2x2GridStrategy();
};
