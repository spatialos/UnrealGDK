// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

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
class UAbstractLBStrategy;
class FLoadBalancingCalculator;
struct FPartitionDeclaration;

namespace SpatialGDK
{
class InterestFactory;

class SpatialStrategySystem
{
public:
	SpatialStrategySystem(const FSubView& InLBView, const FSubView& InWorkerView, const FSubView& InPartitionView,
						  Worker_EntityId InStrategyWorkerEntityId, SpatialOSWorkerInterface* Connection, UAbstractLBStrategy& InStrategy,
						  SpatialVirtualWorkerTranslator& InTranslator, TUniquePtr<InterestFactory>&& InInterestF);

	~SpatialStrategySystem();

	void Advance(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);
	void Destroy(SpatialOSWorkerInterface* Connection);

private:
	void QueryTranslation(SpatialOSWorkerInterface* Connection);

	void CheckPartitionDistributed(SpatialOSWorkerInterface* Connection);

	uint32 NumPartitions;

	const FSubView& LBView;
	const FSubView& WorkerView;
	const FSubView& PartitionView;
	Worker_EntityId StrategyWorkerEntityId;
	Worker_EntityId StrategyPartitionEntityId;
	Worker_RequestId StrategyWorkerRequest;

	TSet<Worker_RequestId> PartitionCreationRequests;

	UAbstractLBStrategy& Strategy;
	SpatialVirtualWorkerTranslator& Translator;
	TUniquePtr<InterestFactory> InterestF;

	TSet<Worker_ComponentId> UpdatesToConsider;
	TSet<Worker_EntityId> MigratingEntities;
	TSet<Worker_EntityId> EntitiesACKMigration;
	TSet<Worker_EntityId> EntitiesClientChanged;
	TMap<Worker_EntityId, AuthorityIntentV2> AuthorityIntentView;
	TMap<Worker_EntityId, AuthorityDelegation> AuthorityDelegationView;
	TMap<Worker_EntityId, NetOwningClientWorker> NetOwningClientView;
	TUniquePtr<FLoadBalancingCalculator> StrategyCalculator;
	TMap<TSharedPtr<FPartitionDeclaration>, uint32> PartitionsMap;

	void SetPartitionReady(Worker_EntityId EntityId);

	struct StrategyPartition
	{
		StrategyPartition(Worker_PartitionId InId)
			: Id(InId)
		{
		}
		Worker_PartitionId Id;
		bool bAcked = false;
	};

	TArray<StrategyPartition> StrategyPartitions;
	bool bStrategyPartitionsCreated = false;

	TMap<Worker_EntityId, ServerWorker> Workers;
	uint32 NumWorkerReady = 0;

	bool bPartitionsDistributed = false;

	Worker_RequestId WorkerTranslationRequest;
	bool bTranslationQueryInFlight = false;
	bool bTranslatorIsReady = false;
};
} // namespace SpatialGDK
