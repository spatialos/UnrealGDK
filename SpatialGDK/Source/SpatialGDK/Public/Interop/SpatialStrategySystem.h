// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LoadBalancingTypes.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/CrossServerEndpoint.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialView/SubView.h"
#include "Utils/CrossServerUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStrategySystem, Log, All)

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class ISpatialOSWorker;
class FLoadBalancingStrategy;
class FPartitionManager;

class FSpatialStrategySystem
{
public:
	FSpatialStrategySystem(TUniquePtr<FPartitionManager> InPartitionMgr, const FSubView& InLBView,
						   TUniquePtr<FLoadBalancingStrategy> Strategy);

	~FSpatialStrategySystem();

	void Advance(ISpatialOSWorker& Connection);
	void Flush(ISpatialOSWorker& Connection);
	void Destroy(ISpatialOSWorker& Connection);

private:
	const FSubView& LBView;

	TUniquePtr<FPartitionManager> PartitionsMgr;

	// +++ Components watched to implement the strategy +++
	TLBDataStorage<AuthorityIntentACK> AuthACKView;
	TLBDataStorage<NetOwningClientWorker> NetOwningClientView;
	FLBDataCollection DataStorages;
	FLBDataCollection UserDataStorages;
	TSet<Worker_ComponentId> UpdatesToConsider;
	// --- Components watched to implement the strategy ---

	// +++ Components managed by the strategy worker +++
	TMap<Worker_EntityId_Key, AuthorityIntentV2> AuthorityIntentView;
	TMap<Worker_EntityId_Key, AuthorityDelegation> AuthorityDelegationView;
	// --- Components managed by the strategy worker ---

	// +++ Migration data +++
	TUniquePtr<FLoadBalancingStrategy> Strategy;
	TSet<Worker_EntityId_Key> MigratingEntities;
	TMap<Worker_EntityId_Key, FPartitionHandle> PendingMigrations;
	// --- Migration data ---

	void UpdateStrategySystemInterest(ISpatialOSWorker& Connection);
	bool bStrategySystemInterestDirty = false;
};
} // namespace SpatialGDK
