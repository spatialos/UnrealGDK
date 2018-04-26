// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AddComponentOpWrapperBase.h"
#include "ComponentAddOpQueueWrapper.generated.h"
#include "CoreMinimal.h"
#include "EngineMinimal.h"

USTRUCT()
struct FComponentAddOpQueueWrapper
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	TArray<UAddComponentOpWrapperBase *> Underlying;

	FComponentAddOpQueueWrapper()
	{
	}
};
