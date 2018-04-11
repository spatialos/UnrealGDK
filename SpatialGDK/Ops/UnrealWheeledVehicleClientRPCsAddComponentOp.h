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
#include "UnrealWheeledVehicleClientRPCsAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealWheeledVehicleClientRPCsAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealWheeledVehicleClientRPCsAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealWheeledVehicleClientRPCsData> Data;
private:
};
