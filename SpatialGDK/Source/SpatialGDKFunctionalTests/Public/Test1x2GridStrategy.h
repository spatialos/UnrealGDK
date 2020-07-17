// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Test1x2GridStrategy.generated.h"

/**
 * A 1 by 2 (rows by columns) load balancing strategy for testing zoning features.
 * Has a 10000 unit interest border, so almost everything should be in view.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2GridStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest1x2GridStrategy();
};
