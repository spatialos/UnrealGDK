// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/ActorSetMember.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentTypes.h"

class USpatialPackageMapClient;

namespace SpatialGDK
{
class ViewCoordinator;

class ActorSetWriter
{
public:
	explicit ActorSetWriter(ViewCoordinator& InCoordinator, const USpatialPackageMapClient& InPackageMap)
		: Coordinator(InCoordinator)
		, PackageMap(InPackageMap)
	{
	}

	ActorSetMember GetActorSetData(AActor* Actor) const;

	void UpdateActorSetComponent(Worker_EntityId ActorEntityId, AActor* Actor) const;

private:
	ViewCoordinator& Coordinator;
	const USpatialPackageMapClient& PackageMap;
};
} // namespace SpatialGDK
