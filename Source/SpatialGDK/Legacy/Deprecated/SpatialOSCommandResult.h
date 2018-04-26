// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// ===========

#pragma once

#include "CommanderTypes.h"
#include "RequestId.h"
#include "UObject/NoExportTypes.h"
#include "SpatialOSCommandResult.generated.h"

USTRUCT(BlueprintType)
struct SPATIALGDK_API FSpatialOSCommandResult
{
	GENERATED_USTRUCT_BODY();

  public:
	FSpatialOSCommandResult()
	: StatusCode(ECommandResponseCode::Unknown)
	{
	}

	bool Success() const
	{
		return StatusCode == ECommandResponseCode::Success;
	}

	FString GetErrorMessage() const
	{
		return ErrorMessage;
	}

	UPROPERTY()
	ECommandResponseCode StatusCode;

	UPROPERTY()
	FString ErrorMessage;

	UPROPERTY()
	FRequestId RequestId;
};