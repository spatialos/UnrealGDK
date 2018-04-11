// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialGDKWorkerTypes.h"
#include "improbable/unreal/generated/UnrealCharacter.h"
#include "UnrealCharacterClientRPCsAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UUnrealCharacterClientRPCsAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UUnrealCharacterClientRPCsAddComponentOp() {}

    ::worker::Option<improbable::unreal::UnrealCharacterClientRPCsData> Data;
private:
};
