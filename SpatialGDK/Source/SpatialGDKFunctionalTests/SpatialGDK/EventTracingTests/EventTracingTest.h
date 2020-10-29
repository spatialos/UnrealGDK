// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/Public/SpatialFunctionalTest.h"

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

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserProcessRPCEventName = "user.process_rpc";

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserReceivePropertyEventName = "user.receive_property";

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserReceiveComponentPropertyEventName = "user.receive_component_property";

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserSendPropertyEventName = "user.send_property";

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserSendComponentPropertyEventName = "user.send_component_property";

	UPROPERTY(BlueprintReadOnly, Category = "EventTracingTest")
	FName UserSendRPCEventName = "user.send_rpc";

protected:

	static FName ReceiveOpEventName;
	static FName SendPropertyUpdatesEventName;
	static FName ReceivePropertyUpdateEventName;
	static FName SendRPCEventName;
	static FName ProcessRPCEventName;
	static FName ComponentUpdateEventName;
	static FName MergeComponentUpdateEventName;

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
