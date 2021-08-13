// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialCommandsHandler.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "LoadBalancing/LoadBalancingStrategy.h"

class SpatialVirtualWorkerTranslator;

namespace SpatialGDK
{
class FSpatialPositionStorage;
class FActorGroupStorage;
class FDirectAssignmentStorage;
class FDebugComponentStorage;
class FCustomWorkerAssignmentStorage;
class FActorSetSystem;
class FEntitySpatialScene;
class FPlayerInterestManager;
class FPlayerControllerData;
class InterestFactory;

class FLegacyLoadBalancing : public FLoadBalancingStrategy
{
public:
	FLegacyLoadBalancing(UAbstractLBStrategy& LegacyLBStrat, SpatialVirtualWorkerTranslator& InTranslator, InterestFactory& InterestF);
	~FLegacyLoadBalancing();

	virtual void Init(FActorInformation& ActorInfo, TArray<FLBDataStorage*>& OutLoadBalancingData,
					  TArray<FLBDataStorage*>& OutServerWorkerData) override;

	virtual void Advance(ISpatialOSWorker& Connection, const FSubView& LBDataSubview) override;
	virtual void Flush(ISpatialOSWorker& Connection) override;

	virtual void OnWorkersConnected(TArrayView<FLBWorkerHandle> ConnectedWorkers) override;
	virtual void OnWorkersDisconnected(TArrayView<FLBWorkerHandle> DisconnectedWorkers) override;
	virtual void TickPartitions(FPartitionManager& Partitions) override;
	virtual void CollectEntitiesToMigrate(FMigrationContext& Ctx) override;

protected:
	void QueryTranslation(ISpatialOSWorker& Connection);
	void EvaluateDebugComponent(Worker_EntityId, FMigrationContext& Ctx);
	TOptional<TPair<Worker_EntityId, uint32>> EvaluateDebugComponent(Worker_EntityId, FActorSetSystem&);
	TOptional<uint32> EvaluateDebugComponent(Worker_EntityId);

	// +++ Data Storage +++
	TUniquePtr<FSpatialPositionStorage> PositionStorage;
	TUniquePtr<FActorGroupStorage> GroupStorage;
	TUniquePtr<FDirectAssignmentStorage> AssignmentStorage;
	TUniquePtr<FDebugComponentStorage> DebugCompStorage;

	TUniquePtr<FCustomWorkerAssignmentStorage> ServerWorkerCustomAssignment;
	// --- Data Storage ---

	// +++ Partition Assignment +++
	TArray<FPartitionHandle> Partitions;
	TArray<FLBWorkerHandle> VirtualWorkerIdToHandle;
	TSet<FLBWorkerHandle> ConnectedWorkers;
	SpatialVirtualWorkerTranslator& Translator;
	FCommandsHandler CommandsHandler;
	TOptional<Worker_RequestId> WorkerTranslationRequest;
	uint32 ExpectedWorkers = 0;
	bool bCreatedPartitions = false;
	bool bTranslatorIsReady = false;
	// --- Partition Assignment ---

	// +++ Load Balancing +++
	FLegacyLBContext LBContext;
	TSet<Worker_EntityId_Key> ToRefresh;
	TMap<Worker_EntityId_Key, int32> Assignment;
	bool bDirectAssignment = false;

	Worker_EntityId WorkerForCustomAssignment = SpatialConstants::INVALID_ENTITY_ID;
	// --- Load Balancing ---

	// +++ EXPERIMENTAL STUFF +++
	TUniquePtr<FEntitySpatialScene> SpatialScene;
	TUniquePtr<FPlayerInterestManager> PlayerInterest;
	FActorInformation* ActorInfo;
	// --- EXPERIMENTAL STUFF ---
};

} // namespace SpatialGDK
