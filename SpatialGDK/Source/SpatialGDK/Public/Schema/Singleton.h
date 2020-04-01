// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
	struct Singleton : Component
	{
		static const Worker_ComponentId ComponentId = SpatialConstants::SINGLETON_COMPONENT_ID;

		Singleton() = default;
		Singleton(const Worker_ComponentData& Data)
		{
		}

		FORCEINLINE Worker_ComponentData CreateSingletonData()
		{
			Worker_ComponentData Data = {};
			Data.component_id = ComponentId;
			Data.schema_type = Schema_CreateComponentData();

			return Data;
		}

		static bool ShouldHaveLocalWorkerAuthority(AActor* SingletonActor, UAbstractLBStrategy* LBStrategy, USpatialStaticComponentView* ComponentView)
		{
			const bool bLoadBalancerEnabled = GetDefault<USpatialGDKSettings>()->bEnableUnrealLoadBalancer;
			// We should check here because if we're calling this before GSM entity data is received it will effectively be giving a false negative.
			check(ComponentView->HasComponent(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID));
			const bool bHasGSMAuthority = ComponentView->HasAuthority(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID, SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID);
			return (bLoadBalancerEnabled && LBStrategy->ShouldHaveAuthority(*SingletonActor)) || (!bLoadBalancerEnabled && bHasGSMAuthority);
		}
	};
} // namespace SpatialGDK
