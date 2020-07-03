// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialSender.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

#include "Containers/Queue.h"
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class UAbstractLBStrategy;

class SPATIALGDK_API SpatialVirtualWorkerTranslator
{
public:
	struct WorkerInformation
	{
		PhysicalWorkerName WorkerName;
		Worker_EntityId ServerWorkerEntityId;
		Worker_PartitionId PartitionEntityId;
	};

	SpatialVirtualWorkerTranslator() = delete;
	SpatialVirtualWorkerTranslator(UAbstractLBStrategy* InLoadBalanceStrategy,
		PhysicalWorkerName InPhysicalWorkerName);

	void SetNetDriver(USpatialNetDriver* InNetDriver) { NetDriver = InNetDriver; };

	// Returns true if the Translator has received the information needed to map virtual workers to physical workers.
	// Currently that is only the number of virtual workers desired.
	bool IsReady() const { return bIsReady; }

	VirtualWorkerId GetLocalVirtualWorkerId() const { return LocalVirtualWorkerId; }
	PhysicalWorkerName GetLocalPhysicalWorkerName() const { return LocalPhysicalWorkerName; }
	Worker_PartitionId GetClaimedPartitionId() const { return LocalPartitionId; }

	// Returns the name of the worker currently assigned to VirtualWorkerId id or nullptr if there is
	// no worker assigned.
	// TODO(harkness): Do we want to copy this data? Otherwise it's only guaranteed to be valid until
	// the next mapping update.
	const PhysicalWorkerName* GetPhysicalWorkerForVirtualWorker(VirtualWorkerId Id) const;
	Worker_PartitionId GetPartitionEntityForVirtualWorker(VirtualWorkerId Id) const;
	Worker_EntityId GetServerWorkerEntityForVirtualWorker(VirtualWorkerId Id) const;

	// On receiving a version of the translation state, apply that to the internal mapping.
	void ApplyVirtualWorkerManagerData(Schema_Object* ComponentObject);

	USpatialNetDriver* NetDriver;

	TWeakObjectPtr<UAbstractLBStrategy> LoadBalanceStrategy;

private:
	TMap<VirtualWorkerId, WorkerInformation> VirtualToPhysicalWorkerMapping;

	bool bIsReady;

	// The WorkerId of this worker, for logging purposes.
	PhysicalWorkerName LocalPhysicalWorkerName;
	VirtualWorkerId LocalVirtualWorkerId;
	Worker_PartitionId LocalPartitionId;

	// Serialization and deserialization of the mapping.
	void ApplyMappingFromSchema(Schema_Object* Object);
	bool IsValidMapping(Schema_Object* Object) const;

	void UpdateMapping(VirtualWorkerId Id, PhysicalWorkerName WorkerName, Worker_PartitionId PartitionEntityId, Worker_EntityId ServerWorkerEntityId);
};
