// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ActorOwnership.h"

#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "EngineClasses/SpatialPackageMapClient.h"

namespace SpatialGDK
{
ActorOwnership ActorOwnership::CreateFromActor(const AActor& Actor, const USpatialPackageMapClient& PackageMap)
{
	ActorOwnership Ownership;
	UNetConnection* OwningConnection = Actor.GetNetConnection();

	if (IsValid(OwningConnection))
	{
		// Add owning PlayerController's EntityId.
		const Worker_EntityId ControllerEntity = PackageMap.GetEntityIdFromObject(OwningConnection->PlayerController);
		check(ControllerEntity != SpatialConstants::INVALID_ENTITY_ID);
		Ownership.OwnerActorEntityId = ControllerEntity;
	}
	else
	{
		// When no player owns an actor, use INVALID_ENTITY_ID.
		Ownership.OwnerActorEntityId = SpatialConstants::INVALID_ENTITY_ID;
	}
	return Ownership;
}
} // namespace SpatialGDK
