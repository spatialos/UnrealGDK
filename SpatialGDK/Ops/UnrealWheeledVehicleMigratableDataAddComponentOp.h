// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/generated/UnrealWheeledVehicle.h"
#include "UnrealWheeledVehicleMigratableDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealWheeledVehicleMigratableDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealWheeledVehicleMigratableDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealWheeledVehicleMigratableDataData> Data;
private:
};
