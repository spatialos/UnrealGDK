// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "Map.h"

#include "WorkerSDK/improbable/trace.h"

#include "SpatialLatencyTracing.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLatencyTracing, Log, All);

class AActor;
class UFunction;
class USpatialNetDriver;

//typedef TPair<const AActor*, const UFunction*> TraceKey;
typedef int32 TraceKey;
//typedef improbable::trace::Span TraceSpan;
struct TraceSpan
{
	AActor* Actor = nullptr;
	UFunction* Function = nullptr;
	improbable::trace::Span Trace;
};

UCLASS()
class SPATIALGDK_API USpatialLatencyTracing : public UObject
{
	GENERATED_BODY()

public:
	// Front-end exposed, allows users to register, start, continue, and end traces
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void RegisterProject(const FString& ProjectId);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool BeginLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool ContinueLatencyTrace(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool EndLatencyTrace();

	// Internal GDK usage, shouldn't be used by game code
	static TraceKey CreateTraceKey(const UObject* Obj, const UFunction* Function);
	static bool IsValidKey(const TraceKey& Key);
	//static TraceSpan* GetTrace(const TraceKey& Key);

	static void AddKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc);
	static void EndLatencyTrace(const TraceSpan* Trace, const FString& TraceDesc);

	static void WriteToSchemaObject(Schema_Object* Obj, const TraceKey& Key);
	static void ReadFromSchemaObject(Schema_Object* Obj);

private:

	static bool CreateTraceKey(const AActor* Actor, const FString& FunctionName, TraceKey& OutKey);
	static TraceSpan* GetActiveTrace();

	static TMap<TraceKey, TraceSpan> TraceMap;

public:

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void SendTestTrace();
};
