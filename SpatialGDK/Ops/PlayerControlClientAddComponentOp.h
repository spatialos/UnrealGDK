// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/player.h"
#include "PlayerControlClientAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UPlayerControlClientAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UPlayerControlClientAddComponentOp() {}

    ::worker::Option<improbable::unreal::PlayerControlClientData> Data;
private:
};
