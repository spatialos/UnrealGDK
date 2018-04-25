// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialOSWorkerTypes.h"
#include "TestSchema.h"
#include "TestFieldTypesAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALOS_API UTestFieldTypesAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UTestFieldTypesAddComponentOp() {}

    ::worker::Option<test::TestFieldTypesData> Data;
private:
};
