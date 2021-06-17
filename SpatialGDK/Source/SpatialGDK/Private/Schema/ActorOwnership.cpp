// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ActorOwnership.h"

#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "EngineClasses/SpatialPackageMapClient.h"

namespace SpatialGDK
{
ActorOwnership::ActorOwnership(const ComponentData& Data)
{
	ApplySchema(Data.GetFields());
}

ActorOwnership::ActorOwnership(const Worker_ComponentData& Data)
{
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

	ApplySchema(ComponentObject);
}

ComponentData ActorOwnership::CreateComponentData() const
{
	return CreateComponentDataHelper(*this);
}

ComponentUpdate ActorOwnership::CreateComponentUpdate() const
{
	return CreateComponentUpdateHelper(*this);
}

void ActorOwnership::ApplyComponentUpdate(const ComponentUpdate& Update)
{
	ApplySchema(Update.GetFields());
}

void ActorOwnership::ApplySchema(Schema_Object* Schema)
{
	if (Schema != nullptr)
	{
		if (Schema_GetEntityIdCount(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID) == 1)
		{
			OwnerActorEntityId = GetEntityIdFromSchema(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID);
		}
	}
}

void ActorOwnership::WriteSchema(Schema_Object* Schema) const
{
	if (Schema != nullptr)
	{
		AddEntityIdToSchema(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID, OwnerActorEntityId);
	}
}

ActorOwnership ActorOwnership::CreateFromActor(const AActor& Actor, const USpatialPackageMapClient& PackageMap)
{
	ActorOwnership Ownership;
	UNetConnection* OwningConnection = Actor.GetNetConnection();

	if (IsValid(OwningConnection))
	{
		// Add owning PlayerController's EntityId.
		const FSpatialEntityId ControllerEntity = PackageMap.GetEntityIdFromObject(OwningConnection->PlayerController);
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
