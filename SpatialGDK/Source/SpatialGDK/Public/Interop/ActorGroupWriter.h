// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorGroupMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"

class UAbstractLBStrategy;

namespace SpatialGDK
{
class ActorGroupWriter
{
public:
	explicit ActorGroupWriter(const UAbstractLBStrategy& InLoadBalancingStrategy)
		: LoadBalancingStrategy(InLoadBalancingStrategy)
	{
	}

	ActorGroupMember GetActorGroupData(AActor* Actor) const;

private:
	const UAbstractLBStrategy& LoadBalancingStrategy;
};
} // namespace SpatialGDK
