// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialVirtualWorkerTranslator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class USpatialNetDriver;

typedef uint32 ZoneId;
typedef uint32 VirtualWorkerId;
typedef FString PhysicalWorkerName;
// typedef TMap<ZoneId, VirtualWorkerId> ZoneToVirtualWorkerMap;
typedef TMap<VirtualWorkerId, PhysicalWorkerName> VirtualToPhysicalWorkerMap;

UCLASS(SpatialType = (Singleton, ServerOnly))
class SPATIALGDK_API USpatialVirtualWorkerTranslator : public UObject {
	GENERATED_UCLASS_BODY()
public:
	void Init(USpatialNetDriver* InNetDriver);

	// Returns the name of the worker currently assigned to VirtualWorkerId id or nullptr if there is
	// no worker assigned.
	// TODO(harkness): Do we want to copy this data? Otherwise it's only guaranteed to be valid until
	// the next mapping update.
	const FString* GetPhysicalWorkerForVirtualWorker(VirtualWorkerId id);

	// On receiving an update to the translation state, apply that to the internal mapping.
	void ApplyVirtualWorkerManagerData(const Worker_ComponentData& Data);

	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);

private:
	USpatialNetDriver* NetDriver;

	VirtualToPhysicalWorkerMap VirtualToPhysicalWorkerMapping;
}; 
