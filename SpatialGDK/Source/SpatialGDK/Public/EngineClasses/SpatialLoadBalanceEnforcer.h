// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/AuthorityIntent.h"
#include "Schema/ComponentPresence.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialCommonTypes.h"

#include "SpatialView/EntityComponentTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialLoadBalanceEnforcer, Log, All)

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class FSubView;

struct LBComponents
{
	EntityAcl Acl;
	AuthorityIntent Intent;
	ComponentPresence Presence;
	NetOwningClientWorker OwningClientWorker;
};

// The load balance enforcer system running on a worker is responsible for updating the state of any ACL
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
	void ShortCircuitMaybeRefreshAcl(const FEntityId EntityId);

private:
	void PopulateDataStore(const FEntityId EntityId);
	bool ApplyComponentUpdate(const FEntityId EntityId, const FComponentId ComponentId, Schema_ComponentUpdate* Update);
	bool ApplyComponentRefresh(const FEntityId EntityId, const FComponentId ComponentId, Schema_ComponentData* Data);

	void RefreshAcl(const FEntityId EntityId);
	EntityComponentUpdate ConstructAclUpdate(const FEntityId EntityId, const PhysicalWorkerName* DestinationWorkerId);

	const PhysicalWorkerName WorkerId;
	const FSubView* SubView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;
	TMap<FEntityId, LBComponents> DataStore;
	TUniqueFunction<void(EntityComponentUpdate)> UpdateSender;
};

} // namespace SpatialGDK
