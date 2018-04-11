// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/unreal/generated/UnrealCharacter.h"
#include "UnrealCharacterMultiClientRepDataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealCharacterMultiClientRepDataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealCharacterMultiClientRepDataAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealCharacterMultiClientRepDataData> Data;
private:
};
