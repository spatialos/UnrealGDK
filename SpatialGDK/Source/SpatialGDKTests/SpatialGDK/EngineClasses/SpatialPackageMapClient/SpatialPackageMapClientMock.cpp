// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClientMock.h"

void USpatialPackageMapClientMock::Init(Worker_EntityId inReturnEntityId)
{
	ReturnEntityId = inReturnEntityId;
}

Worker_EntityId USpatialPackageMapClientMock::GetEntityIdFromObject(const UObject* Object)
{
	return ReturnEntityId;
}
