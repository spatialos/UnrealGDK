// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "Map.h"

#if TRACE_LIB_ACTIVE
#include "WorkerSDK/improbable/trace.h"
#endif

#include "SpatialLatencyTracing.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLatencyTracing, Log, All);

class AActor;
class UFunction;

using TraceKey = int32;

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

	static const TraceKey ActiveTraceKey = 0;
	static const TraceKey InvalidTraceKey = -1;

#if TRACE_LIB_ACTIVE
	// Internal GDK usage, shouldn't be used by game code
	static bool IsValidKey(const TraceKey& Key);
	static TraceKey GetTraceKey(const UObject* Obj, const UFunction* Function);

	static void WriteToLatencyTrace(const TraceKey& Key, const FString& TraceDesc);
	static void EndLatencyTrace(const TraceKey& Key, const FString& TraceDesc);

	static void WriteTraceToSchemaObject(const TraceKey& Key, Schema_Object* Obj);
	static TraceKey ReadTraceFromSchemaObject(Schema_Object* Obj);

private:

	using ActorFuncKey = TPair<const AActor*, const UFunction*>;
	using TraceSpan = improbable::trace::Span;

	static TraceKey CreateNewTraceEntry(const AActor* Actor, const FString& FunctionName);
	static TraceSpan* GetActiveTrace();

	static void WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc);

	static TMap<ActorFuncKey, TraceKey> TrackingTraces;
	static TMap<TraceKey, TraceSpan> TraceMap;

	static FCriticalSection Mutex;

public:

#endif // TRACE_LIB_ACTIVE

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void SendTestTrace();
};
