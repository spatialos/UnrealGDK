// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "Containers/Map.h"

#if TRACE_LIB_ACTIVE
#include "WorkerSDK/improbable/trace.h"
#endif

#include "SpatialLatencyTracer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLatencyTracing, Log, All);

class AActor;
class UFunction;
class USpatialGameInstance;

using TraceKey = int32;

UCLASS()
class SPATIALGDK_API USpatialLatencyTracer : public UObject
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////////////////////////////////
	//
	// USpatialLatencyTracer allows for tracing of gameplay events across multiple workers, from their user
	// instigation, to their observed results. Key timings related to these events are logged throughout
	// the Unreal GDK networking stack.
	// These timings are logged to Google's Stackdriver (https://cloud.google.com/stackdriver/)
	//
	// Setup:
	// 1. Setup a Google project with access to Stackdriver.
	// 2. Create and download a service-account certificate
	// 3. Set GOOGLE_APPLICATION_CREDENTIALS to certificate path
	// 4. Set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH to your `roots.pem` gRPC path
	//
	// Usage:
	// 1. Register your Google's project id with `RegisterProject`
	// 2. Start a latency trace using `BeginLatencyTrace` tagging it against an Actor's RPC
	// 3. During the execution of the tagged RPC either;
	//		- continue the trace using `ContinueLatencyTrace`, again tagging it against another Actor's RPC
	//		- or end the trace using `EndLatencyTrace`
	//
	//////////////////////////////////////////////////////////////////////////

	// Front-end exposed, allows users to register, start, continue, and end traces
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static void RegisterProject(UObject* WorldContextObject, const FString& ProjectId);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool BeginLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool ContinueLatencyTrace(UObject* WorldContextObject, const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool EndLatencyTrace(UObject* WorldContextObject);

	static const TraceKey ActiveTraceKey = 0;
	static const TraceKey InvalidTraceKey = -1;

#if TRACE_LIB_ACTIVE

	// Internal GDK usage, shouldn't be used by game code
	static USpatialLatencyTracer* GetTracer(UObject* WorldContextObject);

	bool IsValidKey(TraceKey Key);
	TraceKey GetTraceKey(const UObject* Obj, const UFunction* Function);

	void WriteToLatencyTrace(const TraceKey Key, const FString& TraceDesc);
	void EndLatencyTrace(const TraceKey Key, const FString& TraceDesc);

	void WriteTraceToSchemaObject(const TraceKey Key, Schema_Object* Obj, const Schema_FieldId FieldId);
	TraceKey ReadTraceFromSchemaObject(Schema_Object* Obj, const Schema_FieldId FieldId);

	void SetWorkerId(const FString& NewWorkerId) { WorkerId = NewWorkerId; }

private:

	using ActorFuncKey = TPair<const AActor*, const UFunction*>;
	using TraceSpan = improbable::trace::Span;

	bool BeginLatencyTrace_Internal(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);
	bool ContinueLatencyTrace_Internal(const AActor* Actor, const FString& FunctionName, const FString& TraceDesc);
	bool EndLatencyTrace_Internal();

	TraceKey CreateNewTraceEntry(const AActor* Actor, const FString& FunctionName);
	TraceSpan* GetActiveTrace();

	void WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc);
	FString FormatMessage(const FString& Message) const;

	FString WorkerId;
	TMap<ActorFuncKey, TraceKey> TrackingTraces;
	TMap<TraceKey, TraceSpan> TraceMap;

	FCriticalSection Mutex;

public:

#endif // TRACE_LIB_ACTIVE

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void SendTestTrace();
};
