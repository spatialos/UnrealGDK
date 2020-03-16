 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/Mocks/SpatialClassInfoManagerMock.h"

void USpatialClassInfoManagerMock::SetComponentIdsForComponentType(ESchemaComponentType ComponentType, TArray<Worker_ComponentId> ComponentIds)
{
	ComponentTypeToIds.Add(ComponentType, ComponentIds);
}

const TArray<Worker_ComponentId>& USpatialClassInfoManagerMock::GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const
{
	return ComponentTypeToIds[ComponentType];
}
