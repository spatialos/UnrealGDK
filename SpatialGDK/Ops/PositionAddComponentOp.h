// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "improbable/standard_library.h"
#include "PositionAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UPositionAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UPositionAddComponentOp() {}

    ::worker::Option<improbable::PositionData> Data;
private:
};
