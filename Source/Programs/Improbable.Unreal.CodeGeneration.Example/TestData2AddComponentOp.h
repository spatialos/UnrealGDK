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
#include "TestData2AddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALOS_API UTestData2AddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UTestData2AddComponentOp() {}

    ::worker::Option<test::TestType2> Data;
private:
};
