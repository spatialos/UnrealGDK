// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialStaticComponentViewMock.h"

void USpatialStaticComponentViewMock::Init(Worker_Authority InAuthority, SpatialGDK::ComponentStorageBase* InComponentStorage)
{
	ReturnAuthority = InAuthority;
	ReturnComponentStorage = InComponentStorage;
}

bool USpatialStaticComponentViewMock::HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	return GetAuthority(EntityId, ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;
}

Worker_Authority USpatialStaticComponentViewMock::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	return ReturnAuthority;
}

SpatialGDK::ComponentStorageBase* USpatialStaticComponentViewMock::GetComponentData(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	return ReturnComponentStorage;
}
