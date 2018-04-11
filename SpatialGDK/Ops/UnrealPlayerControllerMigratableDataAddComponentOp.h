// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/generated/UnrealPlayerController.h"
#include "UnrealPlayerControllerMigratableDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealPlayerControllerMigratableDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealPlayerControllerMigratableDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealPlayerControllerMigratableDataData> Data;
private:
};
