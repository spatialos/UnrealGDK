// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "WorkerSDK/improbable/c_worker.h"

namespace SpatialGDK
{
typedef uint32 FActorLoadBalancingGroupId;

// The LoadBalancingStuff component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct SPATIALGDK_API LoadBalancingStuff : AbstractMutableComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::LOAD_BALANCING_STUFF_COMPONENT_ID;

	LoadBalancingStuff()
		: ActorGroupId(0)
		, ActorSetId(0)
	{
	}

	LoadBalancingStuff(const Worker_ComponentData& Data)
		: LoadBalancingStuff(*Data.schema_type)
	{
	}
	LoadBalancingStuff(Schema_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(&Data);

		ActorGroupId = Schema_GetUint32(ComponentObject, 1);

		ActorSetId = Schema_GetUint32(ComponentObject, 2);
	}

	virtual Worker_ComponentData CreateComponentData() const override
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddUint32(ComponentObject, 1, ActorGroupId);
		Schema_AddUint32(ComponentObject, 2, ActorSetId);

		return Data;
	}

	Worker_ComponentUpdate CreateLoadBalancingStuffUpdate() const
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddUint32(ComponentObject, 1, ActorGroupId);
		Schema_AddUint32(ComponentObject, 2, ActorSetId);

		return Update;
	}

	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		ActorGroupId = Schema_GetUint32(ComponentObject, 1);

		ActorSetId = Schema_GetUint32(ComponentObject, 2);
	}

	FActorLoadBalancingGroupId ActorGroupId;

	uint32 ActorSetId;
};

using LoadBalancingData = LoadBalancingStuff;

} // namespace SpatialGDK
