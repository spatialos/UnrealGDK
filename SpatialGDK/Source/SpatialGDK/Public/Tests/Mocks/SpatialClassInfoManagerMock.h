// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"

#include "SpatialClassInfoManagerMock.generated.h"

UCLASS()
class SPATIALGDK_API USpatialClassInfoManagerMock : public USpatialClassInfoManager
{
	GENERATED_BODY()

public:
	void Init(TMap<ESchemaComponentType, TArray<Worker_ComponentId>> InComponentTypeToIds);

	virtual const TArray<Worker_ComponentId>& GetComponentIdsForComponentType(const ESchemaComponentType ComponentType) const override;

private:
	TMap<ESchemaComponentType, TArray<Worker_ComponentId>> ComponentTypeToIds;
};


