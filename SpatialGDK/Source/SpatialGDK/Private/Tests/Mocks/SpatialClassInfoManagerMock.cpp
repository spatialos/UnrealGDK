 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/Mocks/SpatialClassInfoManagerMock.h"

void USpatialClassInfoManagerMock::Init(TMap<ESchemaComponentType, TArray<Worker_ComponentId>> InComponentTypeToIds)
{
	ComponentTypeToIds = InComponentTypeToIds;
}

const TArray<Worker_ComponentId>& USpatialClassInfoManagerMock::GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const
{
	return ComponentTypeToIds[ComponentType];
}
