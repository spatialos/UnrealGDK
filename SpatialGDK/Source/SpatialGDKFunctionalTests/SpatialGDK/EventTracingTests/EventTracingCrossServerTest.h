// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "EventTracingTest.h"

#include "EventTracingCrossServerTest.generated.h"

namespace worker
{
namespace c
{
struct Trace_Item;
} // namespace c
} // namespace worker

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingCrossServerTest : public AEventTracingTest
{
	GENERATED_BODY()
protected:
	virtual int GetRequiredClients() { return 1; }
	virtual int GetRequiredWorkers() { return 2; }
};
