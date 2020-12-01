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
	AuthorityDelegation Delegation;
	AuthorityIntent Intent;
	ComponentPresence Presence;
	NetOwningClientWorker OwningClientWorker;
};

struct AuthorityStateChange
{
	Worker_EntityId EntityId = 0;
	TArray<Worker_ComponentId> ComponentIds;
	VirtualWorkerId TargetVirtualWorker;
};

// The load balance enforcer system running on a worker is responsible for updating the authority delegation component
// to the workers indicated in the Authority Intent and Net Owning Client Worker components.
//
// The LB components are:
//  - Authority Intent (for authority changes)
//  - Component Presence (to enforce all components exist in the authority delegation component)
//  - Net Owning Client Worker (for client authority changes)
//
// The load balance enforcer's view of the world consists of all entities where the authority delegation component
// is delegated to the worker. The passed subview enforces that any entity seen by the enforcer will have all relevant
// LB components present. Each tick, the enforcer reads the deltas for these entities, and if there any changes for any
// of the LB components calculates whether or not an delegation update needs to be sent, and if so, constructs one and
// sends it on to Spatial. If the same worker is authoritative over the authority intent component, a request to construct
// an delegation update will be short circuited locally.
class SpatialLoadBalanceEnforcer
{
public:
	SpatialLoadBalanceEnforcer(const PhysicalWorkerName& InWorkerId, const FSubView& InSubView,
							   const SpatialVirtualWorkerTranslator* InVirtualWorkerTranslator,
							   TUniqueFunction<void(EntityComponentUpdate)> InUpdateSender);

	void Advance();
	void ShortCircuitMaybeRefreshAuthorityDelegation(const Worker_EntityId EntityId);

private:
	void PopulateDataStore(const Worker_EntityId EntityId);
	bool ApplyComponentUpdate(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	bool ApplyComponentRefresh(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentData* Data);

	void RefreshAuthority(const Worker_EntityId EntityId);
	Worker_ComponentUpdate CreateAuthorityDelegationUpdate(const Worker_EntityId EntityId);

	const PhysicalWorkerName WorkerId;
	const FSubView* SubView;
	const SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;

	TArray<Worker_EntityId> PendingEntityAuthorityChanges;
	TMap<Worker_EntityId_Key, LBComponents> DataStore;
	TUniqueFunction<void(EntityComponentUpdate)> UpdateSender;
};

} // namespace SpatialGDK
