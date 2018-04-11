// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/spawner.h"
#include "PlayerSpawnerAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UPlayerSpawnerAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UPlayerSpawnerAddComponentOp() {}

    ::worker::Option<improbable::unreal::PlayerSpawnerData> Data;
private:
};
