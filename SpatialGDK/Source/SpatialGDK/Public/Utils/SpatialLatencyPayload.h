// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"

#include "SpatialLatencyPayload.generated.h"

USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialLatencyPayload 
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint8> TraceId;

	UPROPERTY()
	TArray<uint8> SpanId;
};
