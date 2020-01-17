// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "Engine/PackageMapClient.h"
#include "UObject/Object.h"

#include "AbstractPackageMapClient.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAbstractPackageMapClient : public UPackageMapClient
{
	GENERATED_BODY()
public:
	virtual Worker_EntityId GetEntityIdFromObject(const UObject* Object) PURE_VIRTUAL(UAbstractPackageMapClient::GetEntityIdFromObject, return SpatialConstants::INVALID_ENTITY_ID;);
};
