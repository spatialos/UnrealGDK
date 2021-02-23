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
struct SPATIALGDK_API ActorGroupMember : AbstractMutableComponent
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

	ActorGroupMember(const Worker_ComponentData& Data)
		: ActorGroupMember(Data.schema_type)
	{
	}

	ActorGroupMember(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);

		ApplySchema(ComponentObject);
	}

	virtual Worker_ComponentData CreateComponentData() const override { return CreateComponentDataHelper(*this); }

	Worker_ComponentUpdate CreateComponentUpdate() const { return CreateActorGroupMemberUpdate(); }

	Worker_ComponentUpdate CreateActorGroupMemberUpdate() const { return CreateComponentUpdateHelper(*this); }

	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		ApplySchema(ComponentObject);
	}

	void ApplySchema(Schema_Object* ComponentObject)
	{
		ActorGroupId = Schema_GetUint32(ComponentObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID);
	}

	void WriteSchema(Schema_Object* ComponentObject) const
	{
		Schema_AddUint32(ComponentObject, SpatialConstants::ACTOR_GROUP_MEMBER_COMPONENT_ACTOR_GROUP_ID, ActorGroupId);
	}

	FActorLoadBalancingGroupId ActorGroupId;
};

} // namespace SpatialGDK
