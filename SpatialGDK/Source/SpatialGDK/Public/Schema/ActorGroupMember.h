// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
using FActorLoadBalancingGroupId = uint32;

// The ActorGroupMember component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct SPATIALGDK_API ActorGroupMember
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ID;

	ActorGroupMember(FActorLoadBalancingGroupId InGroupId)
		: ActorGroupId(InGroupId)
	{
	}

	ActorGroupMember()
		: ActorGroupMember(FActorLoadBalancingGroupId{ 0 })
	{
	}

	ActorGroupMember(const ComponentData& Data) { ApplySchema(Data.GetFields()); }

	ComponentData CreateComponentData() const { return CreateComponentDataHelper(*this); }

	ComponentUpdate CreateComponentUpdate() const { return CreateComponentUpdateHelper(*this); }

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplySchema(Update.GetFields()); }

	void ApplySchema(Schema_Object* ComponentObject)
	{
		if (Schema_GetUint32Count(ComponentObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID) == 1)
		{
			ActorGroupId = Schema_GetUint32(ComponentObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
		}
	}

	void WriteSchema(Schema_Object* ComponentObject) const
	{
		Schema_AddUint32(ComponentObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID, ActorGroupId);
	}

	FActorLoadBalancingGroupId ActorGroupId;
};

} // namespace SpatialGDK
