// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"

#include "SpatialClassInfoManagerMock.generated.h"

UCLASS()
class USpatialClassInfoManagerMock : public USpatialClassInfoManager
{
	GENERATED_BODY()
public:
	void SetComponentIdsForComponentType(const ESchemaComponentType ComponentType, TArray<Worker_ComponentId> ComponentIds);

    const TArray<Worker_ComponentId>& GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const;

private:
	TMap<ESchemaComponentType, TArray<Worker_ComponentId>> ComponentTypeToIds;
}; 
