// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialCommonTypes.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

// The load balance enforcer is responsible for enforcing the authority defined in the authority intent component.
//
// Specifically, the load balance enforcer system running on a worker is responsible for updating the state of any ACL
// component delegated to that worker on any change to any of the load balancing (LB) components which have an effect on
// the ACL.
//
// The LB components are:
//  - Authority Intent (for authority changes)
//  - Component Presence (to enforce all components exist in the write ACL)
//  - Net Owning Client Worker (for client authority changes)
//
// The load balance enforcer's view of the world consists of all entities where the ACL is delegated to the worker.
// The passed subview enforces that any entity seen by the enforcer will have all relevant LB components present.
// Each tick, the enforcer reads the deltas for these entities, and if there any changes for any of the LB components
// calculates whether or not an ACL update needs to be sent, and if so, constructs one and sends it on to Spatial.
// If the same worker is authoritative over the authority intent component, a request to construct an ACL update
// will be short circuited locally.
class SpatialLoadBalanceEnforcer
{
public:
	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const USpatialStaticComponentView* InStaticComponentView,
							   const SpatialGDK::FSubView& InSubView, const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator,
							   TUniqueFunction<void(SpatialGDK::EntityComponentUpdate)> InUpdateSender);

	static bool HandlesComponent(Worker_ComponentId ComponentId);

	void Advance();
	void ShortCircuitMaybeRefreshAcl(const Worker_EntityId EntityId);

private:
	void RefreshAcl(const Worker_EntityId EntityId);

	const PhysicalWorkerName WorkerId;
	// todo: come back to removing
	TWeakObjectPtr<const USpatialStaticComponentView> StaticComponentView;
	const SpatialGDK::FSubView* SubView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	TUniqueFunction<void(SpatialGDK::EntityComponentUpdate)> UpdateSender;
};
