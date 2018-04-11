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
#include "PersistenceAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UPersistenceAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UPersistenceAddComponentOp() {}

    ::worker::Option<improbable::PersistenceData> Data;
private:
};
