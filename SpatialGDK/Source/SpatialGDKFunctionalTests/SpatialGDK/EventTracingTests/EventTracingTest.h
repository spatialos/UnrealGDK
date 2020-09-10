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
	FName ProcessRPCEventName = "unreal_gdk.process_rpc";
	FName PropertyUpdateEventName = "unreal_gdk.property_update";
	FName MergeComponentFieldUpdateEventName = "unreal_gdk.merge_component_field_update";
	FName ReceiveOpEventName = "worker.receive_op";

	FWorkerDefinition WorkerDefinition;
	FName StartTestEventName;
	TArray<FName> FilterEventNames;

	float TestTime = 10.0f;
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
