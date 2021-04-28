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
		const Worker_EntityId ControllerEntity = PackageMap.GetEntityIdFromObject(OwningConnection->PlayerController);
		check(ControllerEntity != SpatialConstants::INVALID_ENTITY_ID);
		Ownership.OwnerActorEntityId = ControllerEntity;
	}
	else
	{
		Ownership.OwnerActorEntityId = SpatialConstants::INVALID_ENTITY_ID;
	}
	return Ownership;
}
} // namespace SpatialGDK
