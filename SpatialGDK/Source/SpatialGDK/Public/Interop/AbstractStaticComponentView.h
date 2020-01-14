// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/AuthorityIntent.h"
#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

class AbstractStaticComponentView 
{
public:
	virtual ~AbstractStaticComponentView() {};
	virtual bool HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const { return false; };
	virtual Worker_Authority GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const { return Worker_Authority::WORKER_AUTHORITY_NOT_AUTHORITATIVE; };
	virtual SpatialGDK::ComponentStorageBase* GetComponentData(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const { return nullptr; };
};
