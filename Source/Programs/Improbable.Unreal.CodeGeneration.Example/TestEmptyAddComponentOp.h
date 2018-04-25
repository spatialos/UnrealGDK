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
#include "TestEmptyAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALOS_API UTestEmptyAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UTestEmptyAddComponentOp() {}

    ::worker::Option<test::TestEmptyData> Data;
private:
};
