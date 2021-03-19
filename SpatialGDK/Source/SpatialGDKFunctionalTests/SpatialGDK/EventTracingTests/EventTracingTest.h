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
	const static FName ReceiveOpEventName;
	const static FName PropertyChangedEventName;
	const static FName ReceivePropertyUpdateEventName;
	const static FName PushRPCEventName;
	const static FName ReceiveRPCEventName;
	const static FName ApplyRPCEventName;
	const static FName ComponentUpdateEventName;
	const static FName UserProcessRPCEventName;
	const static FName UserReceivePropertyEventName;
	const static FName UserReceiveComponentPropertyEventName;
	const static FName UserSendPropertyEventName;
	const static FName UserSendComponentPropertyEventName;
	const static FName UserSendRPCEventName;

	const static FName SendCrossServerRPCName;
	const static FName ReceiveCrossServerRPCName;

	const static FName UserSendCrossServerRPCEventName;
	const static FName UserReceiveCrossServerRPCEventName;

	const static FName UserSendCrossServerPropertyEventName;
	const static FName UserReceiveCrossServerPropertyEventName;

	const static FName ApplyCrossServerRPCName;

	FWorkerDefinition WorkerDefinition;
	TArray<FName> FilterEventNames;

	float TestTime = 8.0f;

	TMap<FString, FName> TraceEvents;
	TMap<FString, TArray<FString>> TraceSpans;

	bool CheckEventTraceCause(const FString& SpanIdString, const TArray<FName>& CauseEventNames, int MinimumCauses = 1) const;

	virtual void FinishEventTraceTest();

	virtual int GetRequiredClients() { return 1; }
	virtual int GetRequiredWorkers() { return 1; }

	struct CheckResult
	{
		int NumTested;
		int NumFailed;
	};
	CheckResult CheckCauses(FName From, FName To) const;

private:
	FDateTime TestStartTime;

	void StartEventTracingTest();
	void WaitForTestToEnd();
	void GatherData();
	void GatherDataFromFile(const FString& FilePath);
};
