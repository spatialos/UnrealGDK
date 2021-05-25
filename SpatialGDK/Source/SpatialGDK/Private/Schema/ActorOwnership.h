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
struct ActorOwnership
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ACTOR_OWNERSHIP_COMPONENT_ID;

	explicit ActorOwnership(Worker_EntityId InLeaderEntityId)
		: OwnerActorEntityId(InLeaderEntityId)
	{
	}

	ActorOwnership()
		: ActorOwnership(Worker_EntityId(SpatialConstants::INVALID_ENTITY_ID))
	{
	}

	explicit ActorOwnership(const ComponentData& Data);

	explicit ActorOwnership(const Worker_ComponentData& Data);

	static ActorOwnership CreateFromActor(const AActor& Actor, const USpatialPackageMapClient& PackageMap);

	ComponentData CreateComponentData() const;

	ComponentUpdate CreateComponentUpdate() const;

	void ApplyComponentUpdate(const ComponentUpdate& Update);

	void ApplySchema(Schema_Object* Schema);

	void WriteSchema(Schema_Object* Schema) const;

	Worker_EntityId OwnerActorEntityId;

	friend bool operator==(const ActorOwnership& Lhs, const ActorOwnership& Rhs)
	{
		return Lhs.OwnerActorEntityId == Rhs.OwnerActorEntityId;
	}

	friend bool operator!=(const ActorOwnership& Lhs, const ActorOwnership& Rhs) { return !(Lhs == Rhs); }
};

} // namespace SpatialGDK
