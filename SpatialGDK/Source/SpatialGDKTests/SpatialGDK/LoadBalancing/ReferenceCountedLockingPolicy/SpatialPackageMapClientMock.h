// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractSpatialPackageMapClient.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "SpatialPackageMapClientMock.generated.h"

UCLASS()
class USpatialPackageMapClientMock : public UAbstractSpatialPackageMapClient
{
	GENERATED_BODY()
public:
	void Init(Worker_EntityId EntityId);

	virtual Worker_EntityId GetEntityIdFromObject(const UObject* Object) override;

private:
	Worker_EntityId EntityId;
}; 
