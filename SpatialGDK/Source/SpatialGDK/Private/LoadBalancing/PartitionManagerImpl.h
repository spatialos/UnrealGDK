// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/PartitionManager.h"

#include "Interop/SpatialCommandsHandler.h"
#include "Interop/Startup/SpatialStartupCommon.h"
#include "LoadBalancing/LBDataStorage.h"
#include "LoadBalancing/PartitionManager.h"
#include "Schema/ServerWorker.h"
#include "Schema/StandardLibrary.h"

namespace SpatialGDK
{
struct FPartitionInternalState
{
	Worker_PartitionId Id = 0;
	QueryConstraint LBConstraint;
	TSet<Worker_ComponentId> CurrentMetadataComponents;
	TArray<ComponentData> PendingMetadataCreation;
	TArray<ComponentUpdate> PendingMetadataUpdates;

	TOptional<Worker_RequestId> CreationRequest;
	TOptional<Worker_RequestId> AssignmentRequest;

	FString DisplayName;
	bool bAcked = false;
	bool bInterestDirty = true;
	bool bPartitionCreated = false;

	FLBWorkerHandle UserAssignment;
	FLBWorkerHandle RequestedAssignment;
	FLBWorkerHandle CurrentAssignment;
};

struct FLBWorkerInternalState
{
	Worker_EntityId SystemWorkerId;
	Worker_EntityId ServerWorkerId;

	FName WorkerType;
	FString FullWorkerName;
};

struct FPartitionManager::Impl
{
	Impl(const FSubView& InServerWorkerView, ViewCoordinator& Coordinator, TUniquePtr<InterestFactory>&& InInterestF);

	void AdvanceView(ISpatialOSWorker& Connection);

	void CheckWorkersConnected(TSet<Worker_EntityId_Key> WorkersToInspect, const TSet<Worker_EntityId_Key>& SystemWorkerModified);
	void WorkerConnected(Worker_EntityId ServerWorkerEntityId);

	bool WaitForPartitionIdAvailable(ISpatialOSWorker& Connection);

	void Flush(ISpatialOSWorker& Connection);

	void SetPartitionReady(Worker_EntityId EntityId);

	TArray<ComponentData> CreatePartitionEntityComponents(const FString& PartitionName, const Worker_EntityId EntityId,
														  const QueryConstraint& LBConstraint);

	static constexpr uint32 k_PartitionsReserveRange = 1024;

	const FSubView& WorkerView;
	const FSubView& SystemWorkerView;
	const FSubView& PartitionView;

	TUniquePtr<InterestFactory> InterestF;

	Worker_EntityId StrategyWorkerEntityId = 0;
	TOptional<FStartupExecutor> StartupExecutor;
	FCommandsHandler CommandsHandler;

	// +++ Partition management data +++
	Worker_EntityId FirstPartitionId = 0;
	Worker_EntityId CurPartitionId = 0;

	TMap<Worker_RequestId_Key, FPartitionHandle> PartitionCreationRequests;
	TOptional<Worker_RequestId> PartitionReserveRequest;

	TMap<Worker_EntityId_Key, FPartitionHandle> IdToPartitionsMap;
	TSet<FPartitionHandle> Partitions;
	// --- Partition management data ---

	// +++ Server worker management data +++
	TLBDataStorage<ServerWorker> WorkersData;
	TLBDataStorage<Worker> SystemWorkersData;
	FLBDataCollection WorkersDispatcher;
	FLBDataCollection SystemWorkersDispatcher;

	TSet<FLBWorkerHandle> ConnectedWorkers;
	TArray<FLBWorkerHandle> ConnectedWorkersThisFrame;
	TArray<FLBWorkerHandle> DisconnectedWorkersThisFrame;
	// --- Server worker management data ---
};

} // namespace SpatialGDK
