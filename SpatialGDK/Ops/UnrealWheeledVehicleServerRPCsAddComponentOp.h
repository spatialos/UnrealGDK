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
#include "UnrealWheeledVehicleServerRPCsAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealWheeledVehicleServerRPCsAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealWheeledVehicleServerRPCsAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealWheeledVehicleServerRPCsData> Data;
private:
};
