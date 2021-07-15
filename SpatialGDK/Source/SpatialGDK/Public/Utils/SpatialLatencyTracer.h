// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Containers/Map.h"
#include "Containers/StaticArray.h"
#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialGDKSpanId.h"
#include "SpatialConstants.h"
#include "SpatialLatencyPayload.h"
#include "Utils/GDKPropertyMacros.h"

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

UCLASS(BlueprintType)
class SPATIALGDK_API USpatialLatencyTracer : public UObject
{
	GENERATED_BODY()

public:
	USpatialLatencyTracer();

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void Setup(const FString& WorkerName);

	// Front-end exposed, allows users to register, start, continue, and end traces

	// Call with your google project id. This must be called before latency trace calls are made.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	void RegisterProject(const FString& ProjectId);

	// Start a latency trace. This will start the latency timer and return you a LatencyPayload object. This payload can then be "continued"
	// via a ContinueLatencyTrace call.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool BeginLatencyTrace(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload);

	// Attach a LatencyPayload to an RPC/Actor pair. The next time that RPC is executed on that Actor, the timings will be measured.
	// You must also send the OutContinuedLatencyPayload as a parameter in the RPC.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool ContinueLatencyTraceRPC(const AActor* Actor, const FString& Function, const FString& TraceDesc,
								 const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// Attach a LatencyPayload to an Property/Actor pair. The next time that Property is executed on that Actor, the timings will be
	// measured. The property being measured should be a FSpatialLatencyPayload and should be set to OutContinuedLatencyPayload.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool ContinueLatencyTraceProperty(const AActor* Actor, const FString& Property, const FString& TraceDesc,
									  const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// Store a LatencyPayload to an Tag/Actor pair. This payload will be stored internally until the user is ready to retrieve it.
	// Use RetrievePayload to retrieve the Payload
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool ContinueLatencyTraceTagged(const AActor* Actor, const FString& Tag, const FString& TraceDesc,
									const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutContinuedLatencyPayload);

	// End a latency trace. This will terminate the trace, and can be called on multiple workers all operating on the same trace but the
	// worker that called BeginLatencyTrace must call this at some point to ensure correct e2e latency timings.
	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	bool EndLatencyTrace(const FSpatialLatencyPayload& LatencyPayLoad);

	void SetWorkerId(const FString& NewWorkerId) { WorkerId = NewWorkerId; }
	void ResetWorkerId();

private:
	bool BeginLatencyTrace_Internal(const FString& TraceDesc, FSpatialLatencyPayload& OutLatencyPayload);
	bool ContinueLatencyTrace_Internal(const AActor* Actor, const FString& Target, ETraceType::Type Type, const FString& TraceDesc,
									   const FSpatialLatencyPayload& LatencyPayload, FSpatialLatencyPayload& OutLatencyPayload);
	bool EndLatencyTrace_Internal(const FSpatialLatencyPayload& LatencyPayload);

	FSpatialGDKSpanId EmitTrace(FString EventType, FString Message, FSpatialGDKSpanId* Causes, uint32 NumCauses);

	TSharedPtr<SpatialGDK::SpatialEventTracer> InternalTracer;
	FString WorkerId;
};
