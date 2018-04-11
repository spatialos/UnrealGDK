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
#include "UnrealWheeledVehicleMultiClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealWheeledVehicleMultiClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealWheeledVehicleMultiClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealWheeledVehicleMultiClientRepDataData> Data;
private:
};
