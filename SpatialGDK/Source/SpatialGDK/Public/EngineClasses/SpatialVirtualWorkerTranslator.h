// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class USpatialNetDriver;

typedef uint32 VirtualWorkerId;
typedef FString PhysicalWorkerName;

class SPATIALGDK_API SpatialVirtualWorkerTranslator
{
public:
	SpatialVirtualWorkerTranslator();

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

	TMap<VirtualWorkerId, PhysicalWorkerName>  VirtualToPhysicalWorkerMapping;

	void ApplyMappingFromSchema(Schema_Object* Object);
	void WriteMappingToSchema(Schema_Object* Object);
}; 
