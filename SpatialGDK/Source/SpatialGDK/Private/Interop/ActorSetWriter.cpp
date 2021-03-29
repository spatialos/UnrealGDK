// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorSetWriter.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
ActorSetMember GetActorSetData(const USpatialPackageMapClient& PackageMap, const AActor& Actor)
{
	const AActor* LeaderActor = GetReplicatedHierarchyRoot(&Actor);
	check(IsValid(LeaderActor));

	const Worker_EntityId LeaderEntityId = PackageMap.GetEntityIdFromObject(LeaderActor);
	check(LeaderEntityId != SpatialConstants::INVALID_ENTITY_ID);

	return ActorSetMember(LeaderEntityId);
}
} // namespace SpatialGDK
