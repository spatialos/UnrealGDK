// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorSetMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"

class USpatialPackageMapClient;

namespace SpatialGDK
{
class ViewCoordinator;

ActorSetMember GetActorSetData(const USpatialPackageMapClient& PackageMap, AActor* Actor);
} // namespace SpatialGDK
