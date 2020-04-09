 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialClassInfoManagerMock.h"

void USpatialClassInfoManagerMock::SetComponentIdsForComponentType(ESchemaComponentType ComponentType, TArray<Worker_ComponentId> ComponentIds)
{
	ComponentTypeToIds.Add(ComponentType, MoveTemp(ComponentIds));
}

const TArray<Worker_ComponentId>& USpatialClassInfoManagerMock::GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const
{
	return ComponentTypeToIds[ComponentType];
}
