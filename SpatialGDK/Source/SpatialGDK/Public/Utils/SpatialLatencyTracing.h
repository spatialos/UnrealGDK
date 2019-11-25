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

typedef TPair<AActor*, UFunction*> TraceKey;
typedef improbable::trace::Span TraceSpan;

UCLASS()
class USpatialLatencyTracing : public UObject
{
	GENERATED_BODY()

public:
	void Init(USpatialNetDriver* InNetDriver);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool BeginLatencyTrace(AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool ContinueLatencyTrace(AActor* Actor, const FString& FunctionName, const FString& TraceDesc);

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static bool EndLatencyTrace();

	UFUNCTION(BlueprintCallable, Category = "SpatialOS")
	static void SendTestTrace();

	static TraceSpan* GetTrace(AActor* Actor, UFunction* Function);
	static TraceSpan* GetTrace(UObject* Actor, UFunction* Function);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	static TMap<TraceKey, TraceSpan> TraceMap;
};

