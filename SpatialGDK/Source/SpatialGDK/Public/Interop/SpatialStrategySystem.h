// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LoadBalancing/LoadBalancingTypes.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/CrossServerEndpoint.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"
#include "SpatialView/SubView.h"
#include "Utils/CrossServerUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStrategySystem, Log, All)

class SpatialOSWorkerInterface;
class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class FLoadBalancingStrategy;
class FLBDataStorage;
class FPartitionManager;

class SpatialStrategySystem
{
public:
	SpatialStrategySystem(TUniquePtr<FPartitionManager> InPartitionMgr, const FSubView& InLBView,
						  TUniquePtr<FLoadBalancingStrategy>&& Strategy);

	~SpatialStrategySystem();

	void Advance(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);
	void Destroy(SpatialOSWorkerInterface* Connection);

private:
	const FSubView& LBView;

	TUniquePtr<FPartitionManager> PartitionsMgr;

	TSet<Worker_ComponentId> UpdatesToConsider;
	TSet<Worker_EntityId> MigratingEntities;
	TSet<Worker_EntityId> EntitiesACKMigration;
	TSet<Worker_EntityId> EntitiesClientChanged;
	TMap<Worker_EntityId, FPartitionHandle> PendingMigrations;
	TArray<FLBDataStorage*> DataStorages;
	TMap<Worker_EntityId, AuthorityIntentV2> AuthorityIntentView;
	TMap<Worker_EntityId, AuthorityDelegation> AuthorityDelegationView;
	TMap<Worker_EntityId, NetOwningClientWorker> NetOwningClientView;
	TUniquePtr<FLoadBalancingStrategy> Strategy;

	void UpdateStrategySystemInterest(SpatialOSWorkerInterface* Connection);

	bool bStrategySystemInterestDirty = false;
};
} // namespace SpatialGDK
