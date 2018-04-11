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
#include "UnrealCharacterServerRPCsAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealCharacterServerRPCsAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealCharacterServerRPCsAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealCharacterServerRPCsData> Data;
private:
};
