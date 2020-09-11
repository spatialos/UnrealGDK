// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"

#include "EventTracingTest.generated.h"

namespace worker
{
namespace c
{
struct Trace_Item;
} // namespace c
} // namespace worker

namespace SpatialGDK
{
class SpatialEventTracer;
} // namespace SpatialGDK

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AEventTracingTest();

	virtual void BeginPlay() override;

protected:

	FName ReceiveOpEventName = "worker.receive_op";

	FWorkerDefinition WorkerDefinition;
	TArray<FName> FilterEventNames;

	float TestTime = 20.0f;
	TMap<FString, FName> TraceEvents;
	TMap<FString, TArray<FString>> TraceSpans;

	bool CheckEventTraceCause(FString SpanIdString, TArray<FName> CauseEventNames, int MinimumCauses = 1);

	virtual void FinishEventTraceTest();

private:
	FDateTime TestStartTime;

	void WaitForTestToEnd();

	void TraceCallback(void* UserData, const Trace_Item* Item);

	SpatialGDK::SpatialEventTracer* GetEventTracer() const;
};
