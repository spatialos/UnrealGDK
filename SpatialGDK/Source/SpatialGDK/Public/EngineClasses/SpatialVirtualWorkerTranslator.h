// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialSender.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "Schema/VirtualWorkerTranslation.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class UAbstractLBStrategy;

class SPATIALGDK_API SpatialVirtualWorkerTranslator
{
public:
	SpatialVirtualWorkerTranslator() = delete;
	SpatialVirtualWorkerTranslator(UAbstractLBStrategy* InLoadBalanceStrategy, USpatialNetDriver* InNetDriver,
								   PhysicalWorkerName InLocalPhysicalWorkerName);

	// Returns true if the Translator has received the information needed to map virtual workers to physical workers.
	// Currently that is only the number of virtual workers desired.
	bool IsReady() const { return bIsReady; }

	VirtualWorkerId GetLocalVirtualWorkerId() const { return LocalVirtualWorkerId; }
	PhysicalWorkerName GetLocalPhysicalWorkerName() const { return LocalPhysicalWorkerName; }
	Worker_PartitionId GetClaimedPartitionId() const { return LocalPartitionId; }
	int32 GetMappingCount() const { return VirtualToPhysicalWorkerMapping.Num(); }

	// Returns the name of the worker currently assigned to VirtualWorkerId id or nullptr if there is
	// no worker assigned.
	// TODO(harkness): Do we want to copy this data? Otherwise it's only guaranteed to be valid until
	// the next mapping update.
	const PhysicalWorkerName* GetPhysicalWorkerForVirtualWorker(VirtualWorkerId Id) const;
	Worker_PartitionId GetPartitionEntityForVirtualWorker(VirtualWorkerId Id) const;
	Worker_EntityId GetServerWorkerEntityForVirtualWorker(VirtualWorkerId Id) const;

	// On receiving a version of the translation state, apply that to the internal mapping.
	void ApplyVirtualWorkerManagerData(const SpatialGDK::VirtualWorkerTranslation& Translation);

	USpatialNetDriver* NetDriver;

	TWeakObjectPtr<UAbstractLBStrategy> LoadBalanceStrategy;

private:
	friend class SpatialVirtualWorkerTranslationManager;

	TMap<VirtualWorkerId, SpatialGDK::VirtualWorkerInfo> VirtualToPhysicalWorkerMapping;

	bool bIsReady;

	// The WorkerId of this worker, for logging purposes.
	PhysicalWorkerName LocalPhysicalWorkerName;
	VirtualWorkerId LocalVirtualWorkerId;
	Worker_PartitionId LocalPartitionId;

	// Deserializing component data to local state.
	void UpdateMapping(const SpatialGDK::VirtualWorkerInfo& VirtualWorkerInfo);
};
