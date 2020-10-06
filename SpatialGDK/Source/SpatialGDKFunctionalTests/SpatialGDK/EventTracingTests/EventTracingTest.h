// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"

#include "EventTracingTest.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEventTracingTest, Log, All);

namespace worker
{
namespace c
{
struct Trace_Item;
} // namespace c
} // namespace worker

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AEventTracingTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AEventTracingTest();

	virtual void PrepareTest() override;

protected:
	FName ReceiveOpEventName = "worker.receive_op";
	FName MergeComponentFieldUpdateEventName = "unreal_gdk.merge_component_field_update";

	FWorkerDefinition WorkerDefinition;
	TArray<FName> FilterEventNames;

	float TestTime = 20.0f;

	TMap<FString, FName> TraceEvents;
	TMap<FString, TArray<FString>> TraceSpans;

	bool CheckEventTraceCause(const FString& SpanIdString, const TArray<FName>& CauseEventNames, int MinimumCauses = 1);

	virtual void FinishEventTraceTest();

private:
	FDateTime TestStartTime;

	void StartEventTracingTest();
	void WaitForTestToEnd();
	void GatherData();
	void GatherDataFromFile(const FString& FilePath);
};
