// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialEventTracingTest.h"

#include "EventTracingCrossServerTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingCrossServerTest : public ASpatialEventTracingTest
{
	GENERATED_BODY()
protected:
	virtual int GetRequiredClients() { return 1; }
	virtual int GetRequiredWorkers() { return 2; }
};
