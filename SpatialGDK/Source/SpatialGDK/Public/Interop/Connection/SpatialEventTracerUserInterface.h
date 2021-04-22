// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/UserSpanId.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Containers/Map.h"

#include <WorkerSDK/improbable/c_trace.h>

#include "SpatialEventTracerUserInterface.generated.h"

class USpatialNetDriver;

DECLARE_DYNAMIC_DELEGATE(FEventTracerRPCDelegate);
DECLARE_DELEGATE_OneParam(FEventTracerAddDataDelegate, SpatialGDK::FSpatialTraceEventDataBuilder&);

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracerUserInterface, Log, All);


// Docs on how to use the interface can be found:
// https://docs.google.com/document/d/1i0fOdeldqeZ9kgBdmYcTD3fCwYXT9pX1RzpTIupgjcg/edit?usp=sharing

UCLASS()
class SPATIALGDK_API USpatialEventTracerUserInterface : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * EXPERIMENTAL
	 * Will trace an event using the input data and associate it with the input SpanId
	 * (This API is subject to change)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static FUserSpanId TraceEvent(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, TMap<FString, FString> Data);

	/**
	 * EXPERIMENTAL
	 * Will trace an event using the input data and associate it with the input SpanId
	 * (This API is subject to change)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static FUserSpanId TraceEventWithCauses(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, TMap<FString, FString> Data,
											const TArray<FUserSpanId>& Causes);

	/**
	 * EXPERIMENTAL
	 * Will ensure that the input SpanId is used to continue the tracing of the RPC flow.
	 * Use the Delegate to call your RPC
	 * (This API is subject to change)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static void TraceRPC(UObject* WorldContextObject, FEventTracerRPCDelegate Delegate, const FUserSpanId& SpanId);

	/**
	 * EXPERIMENTAL
	 * Will ensure that the input SpanId is used to continue the tracing of the property update flow.
	 * The input Object should be the object that contains the property
	 * (This API is subject to change)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static void TraceProperty(UObject* WorldContextObject, UObject* Object, const FUserSpanId& UserSpanId);

	/**
	 * EXPERIMENTAL
	 * Used to get the active SpanId from the GDK. Use this to cause your own trace events.
	 * (This API is subject to change)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static bool GetActiveSpanId(UObject* WorldContextObject, FUserSpanId& OutUserSpanId);

	// ----- C++ Specific API -----

	/**
	 * EXPERIMENTAL
	 * Will trace an event using the input data and associate it with the input SpanId
	 * (This API is subject to change)
	 */
	static FUserSpanId TraceEvent(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, FEventTracerAddDataDelegate AddDataDelegate = {});

	/**
	 * EXPERIMENTAL
	 * Will trace an event using the input data and associate it with the input SpanId
	 * (This API is subject to change)
	 */
	static FUserSpanId TraceEventWithCauses(UObject* WorldContextObject, const FString& EventType, const FString& EventMessage, const TArray<FUserSpanId>& Causes,
		FEventTracerAddDataDelegate AddDataDelegate = {});

private:
	static SpatialGDK::SpatialEventTracer* GetEventTracer(UObject* WorldContextObject);
	static USpatialNetDriver* GetSpatialNetDriver(UObject* WorldContextObject);
};
