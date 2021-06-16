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

	ActorSetMember(FSpatialEntityId InLeaderEntityId)
		: ActorSetId(InLeaderEntityId)
	{
	}

	ActorSetMember()
		: ActorSetMember(FSpatialEntityId(SpatialConstants::INVALID_ENTITY_ID))
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
			ActorSetId = GetEntityIdFromSchema(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID);
		}
	}

	void WriteSchema(Schema_Object* Schema) const
	{
		AddEntityIdToSchema(Schema, SpatialConstants::ACTOR_SET_MEMBER_COMPONENT_LEADER_ENTITY_ID, ActorSetId);
	}

	FSpatialEntityId ActorSetId;
};
} // namespace SpatialGDK
