// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/SpatialTraceEvent.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include <WorkerSDK/improbable/c_trace.h>

#include "SpatialEventTracerUserInterface.generated.h"

DECLARE_DYNAMIC_DELEGATE(FEventTracerDynamicDelegate);

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialEventTracerUserInterface, Log, All);

namespace SpatialGDK
{
class SpatialEventTracer;
}

class USpatialNetDriver;

// Docs on how to use the interface can be found:
// https://docs.google.com/document/d/1i0fOdeldqeZ9kgBdmYcTD3fCwYXT9pX1RzpTIupgjcg/edit?usp=sharing

UCLASS()
class SPATIALGDK_API USpatialEventTracerUserInterface : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static FString CreateSpanId(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static FString CreateSpanIdWithCauses(UObject* WorldContextObject, const TArray<FString>& Causes);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static void TraceEvent(UObject* WorldContextObject, const FString& SpanId, FSpatialTraceEvent SpatialTraceEvent);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static void SetActiveSpanId(UObject* WorldContextObject, FEventTracerDynamicDelegate Delegate, const FString& SpanId);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS|EventTracing", meta = (WorldContext = "WorldContextObject"))
	static void AddLatentSpanId(UObject* WorldContextObject, UObject* Object, const FString& SpanId);

	static void AddSpanIdToStack(UObject* WorldContextObject, const FString& SpanId);
	static void PopSpanIdFromStack(UObject* WorldContextObject);

private:
	static void AddLatentActorSpanId(UObject* WorldContextObject, const AActor& Actor, const FString& SpanId);
	static void AddLatentComponentSpanId(UObject* WorldContextObject, const UActorComponent& Component, const FString& SpanId);

	static SpatialGDK::SpatialEventTracer* GetEventTracer(UObject* WorldContextObject);
	static USpatialNetDriver* GetSpatialNetDriver(UObject* WorldContextObject);
};
