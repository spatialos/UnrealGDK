// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UserSpanId.generated.h"

USTRUCT(Blueprintable)
struct FUserSpanId
{
	GENERATED_BODY()

	FUserSpanId() { Data.Reserve(16); }

	explicit FUserSpanId(const TArray<uint8>& InData)
		: Data(InData)
	{
		Data.Reserve(16);
	}

	UPROPERTY(BlueprintReadOnly, Category = "UserSpanId")
	TArray<uint8> Data;
};
