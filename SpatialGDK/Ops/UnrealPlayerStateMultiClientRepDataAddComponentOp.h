// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/generated/UnrealPlayerState.h"
#include "UnrealPlayerStateMultiClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealPlayerStateMultiClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealPlayerStateMultiClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealPlayerStateMultiClientRepDataData> Data;
private:
};
