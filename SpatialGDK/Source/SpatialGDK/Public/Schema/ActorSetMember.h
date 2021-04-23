// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

class USpatialPackageMapClient;

namespace SpatialGDK
{
// The ActorSetMember component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct SPATIALGDK_API ActorSetMember
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_ID;

	ActorSetMember(Worker_EntityId InLeaderEntityId)
		: ActorSetId(InLeaderEntityId)
	{
	}

	ActorSetMember()
		: ActorSetMember(Worker_EntityId(SpatialConstants::INVALID_ENTITY_ID))
	{
	}

	ActorSetMember(const ComponentData& Data) { ApplySchema(Data.GetFields()); }

	ComponentData CreateComponentData() const { return CreateComponentDataHelper(*this); }

	ComponentUpdate CreateComponentUpdate() const { return CreateComponentUpdateHelper(*this); }

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplySchema(Update.GetFields()); }

	void ApplySchema(Schema_Object* Schema)
	{
		if (Schema_GetEntityIdCount(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID) == 1)
		{
			ActorSetId = Schema_GetEntityId(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID);
		}
	}

	void WriteSchema(Schema_Object* Schema) const
	{
		Schema_AddUint32(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID, ActorSetId);
	}

	Worker_EntityId ActorSetId;
};

// The ActorSetMember component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
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

	static ActorOwnership CreateFromActor(const AActor* Actor, const USpatialPackageMapClient& PackageMap);

	ComponentData CreateComponentData() const { return CreateComponentDataHelper(*this); }

	ComponentUpdate CreateComponentUpdate() const { return CreateComponentUpdateHelper(*this); }

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplySchema(Update.GetFields()); }

	void ApplySchema(Schema_Object* Schema)
	{
		if (Schema_GetEntityIdCount(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID) == 1)
		{
			OwnerActorEntityId = Schema_GetEntityId(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID);
		}
	}

	void WriteSchema(Schema_Object* Schema) const
	{
		Schema_AddUint32(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID, OwnerActorEntityId);
	}

	Worker_EntityId OwnerActorEntityId;
};
} // namespace SpatialGDK
