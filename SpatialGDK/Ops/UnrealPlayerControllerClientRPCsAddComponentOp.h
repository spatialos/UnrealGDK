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
#include "UnrealPlayerControllerClientRPCsAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealPlayerControllerClientRPCsAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealPlayerControllerClientRPCsAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealPlayerControllerClientRPCsData> Data;
private:
};
