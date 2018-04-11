// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialGDKWorkerTypes.h"
#include "improbable/unreal/generated/UnrealWheeledVehicle.h"
#include "UnrealWheeledVehicleSingleClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealWheeledVehicleSingleClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealWheeledVehicleSingleClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealWheeledVehicleSingleClientRepDataData> Data;
private:
};
