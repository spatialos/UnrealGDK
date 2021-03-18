// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorGroupWriter.h"

#include "LoadBalancing/AbstractLBStrategy.h"

namespace SpatialGDK
{
ActorGroupMember GetActorGroupData(const UAbstractLBStrategy& LoadBalancingStrategy, const AActor& Actor)
{
	return ActorGroupMember(LoadBalancingStrategy.GetActorGroupId(Actor));
}
} // namespace SpatialGDK
