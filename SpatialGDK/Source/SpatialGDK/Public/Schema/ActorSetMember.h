// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

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

	ActorSetMember(const Worker_ComponentData& Data)
		: ActorSetMember(Data.schema_type)
	{
	}

	ActorSetMember(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);

		ApplySchema(ComponentObject);
	}

	Worker_ComponentData CreateComponentData() const { return CreateComponentDataHelper(*this); }

	Worker_ComponentUpdate CreateComponentUpdate() const { return CreateActorSetMemberUpdate(); }

	Worker_ComponentUpdate CreateActorSetMemberUpdate() const { return CreateComponentUpdateHelper(*this); }

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		ApplySchema(ComponentObject);
	}

	void ApplySchema(Schema_Object* Schema)
	{
		ActorSetId = Schema_GetEntityId(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID);
	}

	void WriteSchema(Schema_Object* Schema) const
	{
		Schema_AddUint32(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID, ActorSetId);
	}

	Worker_EntityId ActorSetId;
};

} // namespace SpatialGDK
