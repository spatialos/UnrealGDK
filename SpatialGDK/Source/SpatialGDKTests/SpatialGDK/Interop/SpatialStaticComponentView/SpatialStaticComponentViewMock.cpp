// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialStaticComponentViewMock.h"

void USpatialStaticComponentViewMock::Init(TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, Worker_Authority>> InEntityComponentAuthorityMap,
	TMap<Worker_EntityId_Key, TMap<Worker_ComponentId, TUniquePtr<SpatialGDK::ComponentStorageBase>>> InEntityComponentMap)
{
	EntityComponentAuthorityMap = InEntityComponentAuthorityMap;
	EntityComponentMap = InEntityComponentMap;
}

bool USpatialStaticComponentViewMock::HasAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	return GetAuthority(EntityId, ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;
}

Worker_Authority USpatialStaticComponentViewMock::GetAuthority(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	if (const TMap<Worker_ComponentId, Worker_Authority>* ComponentAuthorityMap = EntityComponentAuthorityMap.Find(EntityId))
	{
		if (const Worker_Authority* Authority = ComponentAuthorityMap->Find(ComponentId))
		{
			return *Authority;
		}
	}

	return WORKER_AUTHORITY_NOT_AUTHORITATIVE;
}

SpatialGDK::ComponentStorageBase* USpatialStaticComponentViewMock::GetComponentData(Worker_EntityId EntityId, Worker_ComponentId ComponentId) const
{
	if (const auto* ComponentStorageMap = EntityComponentMap.Find(EntityId))
	{
		if (const TUniquePtr<SpatialGDK::ComponentStorageBase>* Component = ComponentStorageMap->Find(ComponentId))
		{
			return Component->Get();
		}
	}

	return nullptr;
}
