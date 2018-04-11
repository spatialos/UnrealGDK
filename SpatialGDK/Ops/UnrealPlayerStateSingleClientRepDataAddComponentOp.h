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
#include "UnrealPlayerStateSingleClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealPlayerStateSingleClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealPlayerStateSingleClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealPlayerStateSingleClientRepDataData> Data;
private:
};
