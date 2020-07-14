// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialConstants.h"
#include "Containers/Map.h"
#include "SpatialLatencyPayload.h"

#if TRACE_LIB_ACTIVE
#include "WorkerSDK/improbable/trace.h"
#endif

#include "SpatialLatencyTracerData.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLatencyTracing, Log, All);

class AActor;
class UFunction;
class USpatialLatencyTracer;

/**
 * Enum that defined the each of the types a trace can be attached to
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

namespace SpatialGDK
{
struct FOutgoingMessage;

class SPATIALGDK_API SpatialLatencyTracerData
{
	friend class ::USpatialLatencyTracer;

public:

#if TRACE_LIB_ACTIVE

	SpatialLatencyTracerData();

	TraceKey RetrievePendingTrace(const UObject* Obj, const UFunction* Function);
	TraceKey RetrievePendingTrace(const UObject* Obj, const UProperty* Property);
	TraceKey RetrievePendingTrace(const UObject* Obj, const FString& Tag);

	void WriteToLatencyTrace(const TraceKey Key, const FString& TraceDesc);
	void WriteAndEndTrace(const TraceKey Key, const FString& TraceDesc, bool bOnlyEndIfTraceRootIsRemote);

	void WriteTraceToSchemaObject(const TraceKey Key, Schema_Object* Obj, const Schema_FieldId FieldId);
	TraceKey ReadTraceFromSchemaObject(Schema_Object* Obj, const Schema_FieldId FieldId);

	void SetWorkerId(const FString& NewWorkerId) { WorkerId = NewWorkerId; }
	void ResetWorkerId();

	void OnEnqueueMessage(const FOutgoingMessage*);
	void OnDequeueMessage(const FOutgoingMessage*);

private:

	using ActorFuncKey = TPair<const AActor*, const UFunction*>;
	using ActorPropertyKey = TPair<const AActor*, const UProperty*>;
	using ActorTagKey = TPair<const AActor*, FString>;
	using TraceSpan = improbable::trace::Span;

	bool BeginLatencyTrace_Internal(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload);
	bool ContinueLatencyTrace_Internal(const AActor* Actor, const FString& Target, ETraceType::Type Type, const FString& TraceDesc, const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload);
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

#endif // TRACE_LIB_ACTIVE
};

using TracerSharedPtr = TSharedPtr<SpatialLatencyTracerData, ESPMode::ThreadSafe>;

}  // namespace SpatialGDK
