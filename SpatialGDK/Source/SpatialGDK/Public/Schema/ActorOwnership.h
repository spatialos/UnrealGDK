// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

class USpatialPackageMapClient;

namespace SpatialGDK
{
// The ActorOwnership component defines actor's player ownership; OwnerActorEntityId
// points to the PlayerController entity that owns a given actor.
struct SPATIALGDK_API ActorOwnership
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_ID;

	ActorOwnership(Worker_EntityId InLeaderEntityId)
		: OwnerActorEntityId(InLeaderEntityId)
	{
	}

	ActorOwnership()
		: ActorOwnership(Worker_EntityId(SpatialConstants::INVALID_ENTITY_ID))
	{
	}

	ActorOwnership(const ComponentData& Data) { ApplySchema(Data.GetFields()); }

	ActorOwnership(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		ApplySchema(ComponentObject);
	}

	static ActorOwnership CreateFromActor(const AActor& Actor, const USpatialPackageMapClient& PackageMap);

	ComponentData CreateComponentData() const { return CreateComponentDataHelper(*this); }

	ComponentUpdate CreateComponentUpdate() const { return CreateComponentUpdateHelper(*this); }

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplySchema(Update.GetFields()); }

	void ApplySchema(Schema_Object* Schema)
	{
		if (Schema_GetEntityIdCount(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID) == 1)
		{
			OwnerActorEntityId = Schema_GetEntityId(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID);
		}
	}

	void WriteSchema(Schema_Object* Schema) const
	{
		Schema_AddInt64(Schema, SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_OWNER_ACTOR_ID, OwnerActorEntityId);
	}

	Worker_EntityId OwnerActorEntityId;

	friend bool operator==(const ActorOwnership& Lhs, const ActorOwnership& Rhs)
	{
		return Lhs.OwnerActorEntityId == Rhs.OwnerActorEntityId;
	}

	friend bool operator!=(const ActorOwnership& Lhs, const ActorOwnership& Rhs) { return !(Lhs == Rhs); }
};
} // namespace SpatialGDK
