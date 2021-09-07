// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialCommandsHandler.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslationManager, Log, All)

class SpatialOSDispatcherInterface;
class SpatialOSWorkerInterface;

//
// The Translation Manager is responsible for querying SpatialOS for all UnrealWorker worker
// entities and querying the LBStrategy for the number of Virtual Workers needed for load
// balancing. Then the Translation manager creates a mapping from physical worker name
// to virtual worker ID, and writes that to the single Translation entity.
//
// One UnrealWorker is arbitrarily chosen by SpatialOS to be authoritative for the Translation
// entity. This class will execute on that worker and will be idle on all other workers.
//
// This class is currently implemented in the UnrealWorker, but none of the logic must be in
// Unreal. It could be moved to an independent worker in the future in cloud deployments. It
// lives here now for convenience and for fast iteration on local deployments.
//

class SPATIALGDK_API SpatialVirtualWorkerTranslationManager
{
public:
	struct PartitionInfo
	{
		Worker_EntityId PartitionEntityId;
		VirtualWorkerId VirtualWorker;
		Worker_EntityId SimulatingWorkerSystemEntityId;
	};

	SpatialVirtualWorkerTranslationManager(SpatialOSWorkerInterface* InConnection, USpatialNetDriver* NetDriver,
										   SpatialVirtualWorkerTranslator* InTranslator);

	void SetNumberOfVirtualWorkers(const uint32 NumVirtualWorkers);

	const TArray<PartitionInfo>& GetAllPartitions() const { return Partitions; };

	void Advance(const TArray<Worker_Op>& Ops);

	SpatialVirtualWorkerTranslator* Translator;

	using FVirtualToPhysicalWorkerMapping = TMap<VirtualWorkerId, SpatialVirtualWorkerTranslator::WorkerInformation>;

	const FVirtualToPhysicalWorkerMapping& GetVirtualWorkerMapping() const;

private:
	SpatialOSWorkerInterface* Connection;
	USpatialNetDriver* NetDriver;

	TArray<VirtualWorkerId> VirtualWorkersToAssign;
	TArray<PartitionInfo> Partitions;
	FVirtualToPhysicalWorkerMapping VirtualToPhysicalWorkerMapping;
	uint32 NumVirtualWorkers;

	bool bWorkerEntityQueryInFlight;

	SpatialGDK::FCommandsHandler CommandsHandler;
};
