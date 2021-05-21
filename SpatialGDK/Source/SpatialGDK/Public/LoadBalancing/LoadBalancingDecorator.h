// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

#include "ReplicationGraphTypes.h"

namespace SpatialGDK
{
class SPATIALGDK_API FLoadBalancingDecorator
{
public:
	virtual ~FLoadBalancingDecorator() = default;

	virtual TArray<SpatialGDK::ComponentData> OnCreate(AActor* Actor) = 0;

	virtual void UpdateDecoration(TMap<Worker_EntityId_Key, SpatialGDK::ComponentUpdate>& OutUpdates) {}
};

class SPATIALGDK_API FLayerLoadBalancingDecorator : public FLoadBalancingDecorator
{
public:
	FLayerLoadBalancingDecorator(TClassMap<uint32>&& ClassToGroup);

	virtual TArray<SpatialGDK::ComponentData> OnCreate(AActor* Actor) override;

protected:
	TClassMap<uint32> ClassToGroup;
};
} // namespace SpatialGDK
