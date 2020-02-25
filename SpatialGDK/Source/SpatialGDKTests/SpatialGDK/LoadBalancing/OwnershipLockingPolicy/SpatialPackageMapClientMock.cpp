// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClientMock.h"

void USpatialPackageMapClientMock::Init(Worker_EntityId InEntityId)
{
	EntityId = InEntityId;
}

Worker_EntityId USpatialPackageMapClientMock::GetEntityIdFromObject(const UObject* Object)
{
	return EntityId;
}
