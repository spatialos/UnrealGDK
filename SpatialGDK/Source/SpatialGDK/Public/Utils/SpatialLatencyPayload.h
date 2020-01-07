// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"

#include "SpatialLatencyPayload.generated.h"

USTRUCT()
struct SPATIALGDK_API FSpatialLatencyPayload 
{
	GENERATED_BODY()

	FSpatialLatencyPayload() {}

	FSpatialLatencyPayload(TArray<uint8>&& TraceBytes, TArray<uint8>&& SpanBytes)
		: TraceId(MoveTemp(TraceBytes))
		, SpanId(MoveTemp(SpanBytes))
	{}

	UPROPERTY()
	TArray<uint8> TraceId;

	UPROPERTY()
	TArray<uint8> SpanId;
};
