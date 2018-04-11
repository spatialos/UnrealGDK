// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialGDKWorkerTypes.h"
#include "improbable/unreal/generated/UnrealPlayerController.h"
#include "UnrealPlayerControllerSingleClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealPlayerControllerSingleClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealPlayerControllerSingleClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealPlayerControllerSingleClientRepDataData> Data;
private:
};
