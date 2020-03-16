 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialClassInfoManagerMock.h"

void USpatialClassInfoManagerMock::SetComponentIdsForComponentType(const ESchemaComponentType ComponentType, TArray<Worker_ComponentId> ComponentIds)
{
    ComponentTypeToIds.Add(ComponentType, ComponentIds);
}

const TArray<Worker_ComponentId>& USpatialClassInfoManagerMock::GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const
{
	return ComponentTypeToIds[ComponentType];
}
