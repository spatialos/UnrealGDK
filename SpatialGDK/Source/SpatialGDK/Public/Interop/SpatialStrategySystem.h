// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Interop/SkeletonEntityManifestPublisher.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "LoadBalancing/ActorSetSystem.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/LoadBalancingTypes.h"
#include "Schema/ActorSetMember.h"
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

struct FStrategySystemViews
{
	const FSubView& LBView;
	const FSubView& ServerWorkerView;
	const FSubView& SkeletonManifestView;
	const FSubView& FilledManifestSubView;
};

class FSpatialStrategySystem
{
public:
	FSpatialStrategySystem(TUniquePtr<FPartitionManager> InPartitionMgr, FStrategySystemViews InViews,
						   TUniquePtr<FLoadBalancingStrategy> Strategy, TUniquePtr<InterestFactory> InInterestF);

	~FSpatialStrategySystem();

	void Init(ISpatialOSWorker& Connection);

	void Advance(ISpatialOSWorker& Connection);
	void Flush(ISpatialOSWorker& Connection);
	void Destroy(ISpatialOSWorker& Connection);

private:
	FStrategySystemViews Views;

	TUniquePtr<FPartitionManager> PartitionsMgr;
	TUniquePtr<InterestFactory> InterestF;

	// +++ Components watched to implement the strategy +++
	TLBDataStorage<AuthorityIntentACK> AuthACKView;
	TLBDataStorage<NetOwningClientWorker> NetOwningClientView;
	TLBDataStorage<ActorSetMember> SetMemberView;
	FActorSetSystem ActorSetSystem;
	FSkeletonManifestPublisher ManifestPublisher;
	FLBDataCollection DataStorages;
	FLBDataCollection UserDataStorages;
	FLBDataCollection ServerWorkerDataStorages;
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
	TMap<Worker_EntityId_Key, FPartitionHandle> EntityAssignment;
	// --- Migration data ---

	void UpdateStrategySystemInterest(ISpatialOSWorker& Connection);
	bool bStrategySystemInterestDirty = false;
};
} // namespace SpatialGDK
