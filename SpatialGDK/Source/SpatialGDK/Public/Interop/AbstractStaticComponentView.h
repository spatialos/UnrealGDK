// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/AuthorityIntent.h"
#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "UObject/Object.h"

#include "AbstractStaticComponentView.generated.h"

UCLASS()
class SPATIALGDK_API UAbstractStaticComponentView : public UObject
{
	GENERATED_BODY()

public:
	virtual bool HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const PURE_VIRTUAL(UAbstractPackageMapClient::GetEntityIdFromObject, return false;);
	virtual Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const  PURE_VIRTUAL(UAbstractPackageMapClient::GetEntityIdFromObject, return Worker_Authority::WORKER_AUTHORITY_NOT_AUTHORITATIVE;);
	virtual SpatialGDK::ComponentStorageBase* GetComponentData(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const PURE_VIRTUAL(UAbstractPackageMapClient::GetEntityIdFromObject, return nullptr;);
};
