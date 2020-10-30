// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialTraceEvent.generated.h"

USTRUCT(Blueprintable)
struct FTraceData
{
	GENERATED_BODY()

	FTraceData(){};
	explicit FTraceData(FString InKey, FString InValue)
		: Key(MoveTemp(InKey))
		, Value(MoveTemp(InValue))
	{
	}

	UPROPERTY(BlueprintReadWrite, Category = "TraceData")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "TraceData")
	FString Value;
};

USTRUCT(Blueprintable)
struct FSpatialTraceEvent
{
	GENERATED_BODY()

	FSpatialTraceEvent(){};
	explicit FSpatialTraceEvent(FString InType, FString InMessage)
		: Type(MoveTemp(InType))
		, Message(MoveTemp(InMessage))
	{
	}

	void AddData(FString Key, FString Value) { Data.Add(FTraceData(MoveTemp(Key), MoveTemp(Value))); }

	UPROPERTY(BlueprintReadWrite, Category = "SpatialTraceEvent")
	FString Type;

	UPROPERTY(BlueprintReadWrite, Category = "SpatialTraceEvent")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "SpatialTraceEvent")
	TArray<FTraceData> Data;
};
