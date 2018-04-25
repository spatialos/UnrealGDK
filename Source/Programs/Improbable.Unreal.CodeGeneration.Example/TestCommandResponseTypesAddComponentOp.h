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
#include "TestCommandResponseTypesAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALOS_API UTestCommandResponseTypesAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UTestCommandResponseTypesAddComponentOp() {}

    ::worker::Option<test::TestCommandResponseTypesData> Data;
private:
};
