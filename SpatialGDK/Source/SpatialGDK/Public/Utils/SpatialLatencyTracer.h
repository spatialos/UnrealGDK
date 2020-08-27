// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Containers/Map.h"
#include "Containers/StaticArray.h"
#include "SpatialConstants.h"
#include "SpatialLatencyPayload.h"
#include "Utils/GDKPropertyMacros.h"

#if TRACE_LIB_ACTIVE
#include "WorkerSDK/improbable/trace.h"
#endif

#include "SpatialLatencyTracer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLatencyTracing, Log, All);

class AActor;
class UFunction;
class USpatialGameInstance;

namespace SpatialGDK
{
struct FOutgoingMessage;
} // namespace SpatialGDK

/**
 * Enum that maps Unreal's log verbosity to allow use in settings.
 **/
UENUM()
namespace ETraceType
{
enum Type
{
	RPC,
	Property,
	Tagged
};
}

UCLASS()
class SPATIALGDK_API USpatialLatencyTracer : public UObject
{
	GENERATED_BODY()

public:
	//////////////////////////////////////////////////////////////////////////
	//
	// EXPERIMENTAL: We do not support this functionality currently: Do not use it unless you are Improbable staff.
	//
	// USpatialLatencyTracer allows for tracing of gameplay events across multiple workers, from their user
	// instigation, to their observed results. Each of these multi-worker events are tracked through `traces`
	// which allow the user to see collected timings of these events in a single location. Key timings related
	// to these events are logged throughout the Unreal GDK networking stack. This API makes the assumption
	// that the distributed workers have had their clocks synced by some time syncing protocol (eg. NTP). To
	// give accurate timings, the trace payload is embedded directly within the relevant networking component
	// updates. This framework also assumes that the worker that calls BeginLatencyTrace will also eventually
	// call EndLatencyTrace on the trace. This allows accurate end-to-end timings.
	//
	// These timings are logged to Google's Stackdriver (https://cloud.google.com/stackdriver/)
	//
	// Setup:
	// 1. Run UnrealGDK SetupIncTraceLibs.bat to include latency tracking libraries.
	// 2. Setup a Google project with access to Stackdriver.
	// 3. Create and download a service-account certificate
	// 4. Set an environment variable GOOGLE_APPLICATION_CREDENTIALS to certificate path
	// 5. Set an environment variable GRPC_DEFAULT_SSL_ROOTS_FILE_PATH to your `roots.pem` gRPC path
	//
	// Usage:
	// 1. Register your Google's project id with `RegisterProject`
	// 2. Start a latency trace using `BeginLatencyTrace` and store the returned payload.
	// 3. Pass this payload to a variant of `ContinueLatencyTrace` depending on how you want to continue the trace (rpc/property/tag)
	//	- If continuing via an RPC include the FSpatialLatencyPayload as an RPC parameter
	//  - If continuing via a Property ensure the property is of type FSpatialLatencyPayload
	// 4. Repeat (3) until the trace is returned to the originating worker.
	// 5. Call `EndLatencyTrace` on the returned payload.
	//
	//////////////////////////////////////////////////////////////////////////

	USpatialLatencyTracer();

	// Front-end exposed, allows users to register, start, continue, and end traces

	// Call with your google project id. This must be called before latency trace calls are made.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static void RegisterProject(UObject* WorldContextObject, const FString& ProjectId);

	// Set metadata string to be included in all span names. Resulting uploaded span names are of the format "USER_SPECIFIED_NAME (METADATA
	// : WORKER_ID)".
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool SetTraceMetadata(UObject* WorldContextObject, const FString& NewTraceMetadata);

	// Start a latency trace. This will start the latency timer and return you a LatencyPayload object. This payload can then be "continued"
	// via a ContinueLatencyTrace call.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool BeginLatencyTrace(UObject* WorldContextObject, const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload);

	// Attach a LatencyPayload to an RPC/Actor pair. The next time that RPC is executed on that Actor, the timings will be measured.
	// You must also send the OutContinuedLatencyPayload as a parameter in the RPC.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool ContinueLatencyTraceRPC(UObject* WorldContextObject, const AActor* Actor, const FString& Function, const FString& TraceDesc,
										const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// Attach a LatencyPayload to an Property/Actor pair. The next time that Property is executed on that Actor, the timings will be
	// measured. The property being measured should be a FSpatialLatencyPayload and should be set to OutContinuedLatencyPayload.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool ContinueLatencyTraceProperty(UObject* WorldContextObject, const AActor* Actor, const FString& Property,
											 const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload,
											 FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// Store a LatencyPayload to an Tag/Actor pair. This payload will be stored internally until the user is ready to retrieve it.
	// Use RetrievePayload to retrieve the Payload
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool ContinueLatencyTraceTagged(UObject* WorldContextObject, const AActor* Actor, const FString& Tag, const FString& TraceDesc,
										   const FSpatialLatencyPayload& LatencyPayload,
										   FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// End a latency trace. This will terminate the trace, and can be called on multiple workers all operating on the same trace but the
	// worker that called BeginLatencyTrace must call this at some point to ensure correct e2e latency timings.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static bool EndLatencyTrace(UObject* WorldContextObject, const FSpatialLatencyPayload& LatencyPayLoad);

	// Returns a previously saved payload from ContinueLatencyTraceTagged
	UFUNCTION(BlueprintCallable, Category = "SpatialOS", meta = (WorldContext = "WorldContextObject"))
	static FSpatialLatencyPayload RetrievePayload(UObject* WorldContextObject, const AActor* Actor, const FString& Tag);

	// Internal GDK usage, shouldn't be used by game code
	static USpatialLatencyTracer* GetTracer(UObject* WorldContextObject);

#if TRACE_LIB_ACTIVE

	bool IsValidKey(TraceKey Key);
	TraceKey RetrievePendingTrace(const UObject* Obj, const UFunction* Function);
	TraceKey RetrievePendingTrace(const UObject* Obj, const GDK_PROPERTY(Property) * Property);
	TraceKey RetrievePendingTrace(const UObject* Obj, const FString& Tag);

	void WriteToLatencyTrace(const TraceKey Key, const FString& TraceDesc);
	void WriteAndEndTrace(const TraceKey Key, const FString& TraceDesc, bool bOnlyEndIfTraceRootIsRemote);

	void WriteTraceToSchemaObject(const TraceKey Key, Schema_Object* Obj, const Schema_FieldId FieldId);
	TraceKey ReadTraceFromSchemaObject(Schema_Object* Obj, const Schema_FieldId FieldId);

	void SetWorkerId(const FString& NewWorkerId) { WorkerId = NewWorkerId; }
	void ResetWorkerId();

	void OnEnqueueMessage(const SpatialGDK::FOutgoingMessage*);
	void OnDequeueMessage(const SpatialGDK::FOutgoingMessage*);

private:
	using ActorFuncKey = TPair<const AActor*, const UFunction*>;
	using ActorPropertyKey = TPair<const AActor*, const GDK_PROPERTY(Property)*>;
	using ActorTagKey = TPair<const AActor*, FString>;
	using TraceSpan = improbable::trace::Span;

	bool BeginLatencyTrace_Internal(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload);
	bool ContinueLatencyTrace_Internal(const AActor* Actor, const FString& Target, ETraceType::Type Type, const FString& TraceDesc,
									   const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload);
	bool EndLatencyTrace_Internal(const FSpatialLatencyPayload& LatencyPayload);

	FSpatialLatencyPayload RetrievePayload_Internal(const UObject* Actor, const FString& Key);

	bool AddTrackingInfo(const AActor* Actor, const FString& Target, const ETraceType::Type Type, const TraceKey Key);

	TraceKey GenerateNewTraceKey();
	void ResolveKeyInLatencyPayload(FSpatialLatencyPayload& Payload);

	void WriteKeyFrameToTrace(const TraceSpan* Trace, const FString& TraceDesc);
	FString FormatMessage(const FString& Message, bool bIncludeMetadata = false) const;

	FString WorkerId;
	FString TraceMetadata;

	TraceKey NextTraceKey = 1;

	FCriticalSection Mutex; // This mutex is to protect modifications to the containers below

	TMap<ActorFuncKey, TraceKey> TrackingRPCs;
	TMap<ActorPropertyKey, TraceKey> TrackingProperties;
	TMap<ActorTagKey, TraceKey> TrackingTags;
	TMap<TraceKey, TraceSpan> TraceMap;

	TSet<TraceKey> RootTraces;

public:
#endif // TRACE_LIB_ACTIVE

	// Used for testing trace functionality, will send a debug trace in three parts from this worker
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void Debug_SendTestTrace();
};
