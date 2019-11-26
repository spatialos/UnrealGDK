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

typedef TPair<const AActor*, const UFunction*> ActorFuncTrack;
typedef int32 TraceKey;
typedef improbable::trace::Span TraceSpan;

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
	static bool IsValidKey(const TraceKey& Key);
	static TraceKey GetTraceKey(const UObject* Obj, const UFunction* Function);

	static void WriteToLatencyTrace(const TraceKey& Key, const FString& TraceDesc);
	static void EndLatencyTrace(const TraceKey& Key, const FString& TraceDesc);

	static void WriteToSchemaObject(Schema_Object* Obj, const TraceKey& Key);
	static TraceKey ReadFromSchemaObject(Schema_Object* Obj);

	static const TraceKey ActiveTraceKey = 0;
	static const TraceKey InvalidTraceKey = -1;

private:

	static TraceKey CreateNewTraceEntry(const AActor* Actor, const FString& FunctionName);
	static TraceSpan* GetActiveTrace();

	static void WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc);

	static TMap<ActorFuncTrack, TraceKey> TrackingTraces;
	static TMap<TraceKey, TraceSpan> TraceMap;

	static FCriticalSection Mutex;

public:

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void SendTestTrace();
};
