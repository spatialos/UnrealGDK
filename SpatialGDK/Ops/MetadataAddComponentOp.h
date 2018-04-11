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
#include "MetadataAddComponentOp.generated.h"

/**
*
*/
UCLASS()
class SPATIALGDK_API UMetadataAddComponentOp : public UAddComponentOpWrapperBase
{
	GENERATED_BODY()

public:
	UMetadataAddComponentOp() {}

    ::worker::Option<improbable::MetadataData> Data;
private:
};
