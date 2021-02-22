// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
template <class TComponent>
Worker_ComponentUpdate CreateComponentUpdateHelper(const TComponent& Component)
{
	Worker_ComponentUpdate Update = {};
	Update.component_id = TComponent::ComponentId;
	Update.schema_type = Schema_CreateComponentUpdate();
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

	Component.WriteSchema(ComponentObject);

	return Update;
}

template <class TComponent>
Worker_ComponentData CreateComponentDataHelper(const TComponent& Component)
{
	Worker_ComponentData Update = {};
	Update.component_id = TComponent::ComponentId;
	Update.schema_type = Schema_CreateComponentData();
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(Update.schema_type);

	Component.WriteSchema(ComponentObject);

	return Update;
}

typedef uint32 FActorLoadBalancingGroupId;

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

// The ActorSetMember component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct SPATIALGDK_API ActorSetMember : AbstractMutableComponent
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

	virtual Worker_ComponentData CreateComponentData() const override { return CreateComponentDataHelper(*this); }

	Worker_ComponentUpdate CreateComponentUpdate() const { return CreateActorSetMemberUpdate(); }

	Worker_ComponentUpdate CreateActorSetMemberUpdate() const { return CreateComponentUpdateHelper(*this); }

	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override
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
