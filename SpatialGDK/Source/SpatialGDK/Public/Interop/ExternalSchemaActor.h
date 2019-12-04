// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "ExternalSchemaActor.generated.h"

UINTERFACE(MinimalAPI)
class UExternalSchemaActor : public UInterface
{
	GENERATED_BODY()
};

class SPATIALGDK_API IExternalSchemaActor
{
	GENERATED_BODY()

public:
	virtual void GetWriteAclMap(WriteAclMap& OutWriteAclMap) const PURE_VIRTUAL(IExternalSchemaActor::GetWriteAclMap, );
	virtual void GetInitialComponentData(TArray<Worker_ComponentData>& OutComponentData) const PURE_VIRTUAL(IExternalSchemaActor::GetInitialComponentData, );
};
