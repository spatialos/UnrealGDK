// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UserSpanId.generated.h"

USTRUCT(Blueprintable)
struct FUserSpanId
{
	GENERATED_BODY()

	FUserSpanId() {}
	explicit FUserSpanId(const TArray<uint8>& InData)
		: Data(InData)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "UserSpanId")
	TArray<uint8> Data;

	bool IsValid() const { return Data.Num() == 16; }
};
