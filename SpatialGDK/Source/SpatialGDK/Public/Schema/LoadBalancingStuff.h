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

	LoadBalancingStuff(const Worker_ComponentData& Data);
	LoadBalancingStuff(Schema_ComponentData& Data);

	virtual Worker_ComponentData CreateComponentData() const override;

	Worker_ComponentUpdate CreateLoadBalancingStuffUpdate() const;

	virtual void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	FActorLoadBalancingGroupId ActorGroupId;

	uint32 ActorSetId;
};

using LoadBalancingData = LoadBalancingStuff;

} // namespace SpatialGDK
