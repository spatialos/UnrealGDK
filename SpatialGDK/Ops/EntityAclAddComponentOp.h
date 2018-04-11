// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AddComponentOpWrapperBase.h"
#include "SpatialGDKWorkerTypes.h"
#include "improbable/standard_library.h"
#include "EntityAclAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UEntityAclAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UEntityAclAddComponentOp() {}

    ::worker::Option<improbable::EntityAclData> Data;
private:
};
