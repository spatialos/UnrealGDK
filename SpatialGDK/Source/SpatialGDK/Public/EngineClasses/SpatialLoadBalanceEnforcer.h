// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialCommonTypes.h"

#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
struct LBComponents
{
	EntityAcl Acl;
	AuthorityIntent Intent;
	ComponentPresence Presence;
	NetOwningClientWorker OwningClientWorker;
};

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
	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const FSubView& InSubView,
							   const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator,
							   TUniqueFunction<void(EntityComponentUpdate)> InUpdateSender);

	void Advance();
	void ShortCircuitMaybeRefreshAcl(const Worker_EntityId EntityId);

private:
	void PopulateDataStore(const Worker_EntityId EntityId);
	bool ApplyComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	bool ApplyComponentRefresh(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void RefreshAcl(const Worker_EntityId EntityId);
	EntityComponentUpdate ConstructAclUpdate(const Worker_EntityId EntityId, const PhysicalWorkerName* DestinationWorkerId);

	const PhysicalWorkerName WorkerId;
	const FSubView* SubView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;
	TMap<Worker_EntityId_Key, LBComponents> DataStore;
	TUniqueFunction<void(EntityComponentUpdate)> UpdateSender;
};

} // namespace SpatialGDK
