// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorGroupMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"

class UAbstractLBStrategy;

namespace SpatialGDK
{
ActorGroupMember GetActorGroupData(const UAbstractLBStrategy& LoadBalancingStrategy, const AActor& Actor);
} // namespace SpatialGDK
