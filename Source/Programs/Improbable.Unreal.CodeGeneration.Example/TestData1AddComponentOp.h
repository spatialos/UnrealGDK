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
#include "TestData1AddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALOS_API UTestData1AddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UTestData1AddComponentOp() {}

    ::worker::Option<test::TestType1> Data;
private:
};
