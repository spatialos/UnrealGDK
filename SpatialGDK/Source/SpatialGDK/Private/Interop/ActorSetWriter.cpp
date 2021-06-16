// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorSetWriter.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
ActorSetMember GetActorSetData(const USpatialPackageMapClient& PackageMap, const AActor& Actor)
{
	const AActor* LeaderActor = GetReplicatedHierarchyRoot(&Actor);

	if (!ensureAlwaysMsgf(IsValid(LeaderActor), TEXT("Failed to get replicated hierarchy root when getting Actor set data for Actor: %s"),
						  *GetNameSafe(&Actor)))
	{
		return ActorSetMember();
	}

	const FSpatialEntityId LeaderEntityId = PackageMap.GetEntityIdFromObject(LeaderActor);

	if (!ensureAlwaysMsgf(LeaderEntityId != SpatialConstants::INVALID_ENTITY_ID,
						  TEXT("Failed to get entity Id from package map for Actor: %s"), *GetNameSafe(LeaderActor)))
	{
		return ActorSetMember();
	}

	return ActorSetMember(LeaderEntityId);
}
} // namespace SpatialGDK
