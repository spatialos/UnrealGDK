// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Queue.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslationManager, Log, All)

class SpatialVirtualWorkerTranslator;
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
	SpatialVirtualWorkerTranslationManager(SpatialOSDispatcherInterface* InReceiver, SpatialOSWorkerInterface* InConnection,
										   SpatialVirtualWorkerTranslator* InTranslator);

	void SetNumberOfVirtualWorkers(const uint32 NumVirtualWorkers);

	// The translation manager only cares about changes to the authority of the translation mapping.
	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);

private:
	SpatialOSDispatcherInterface* Receiver;
	SpatialOSWorkerInterface* Connection;

	SpatialVirtualWorkerTranslator* Translator;

	TMap<VirtualWorkerId, TPair<PhysicalWorkerName, Worker_EntityId>> VirtualToPhysicalWorkerMapping;
	TMap<PhysicalWorkerName, VirtualWorkerId> PhysicalToVirtualWorkerMapping;
	TQueue<VirtualWorkerId> UnassignedVirtualWorkers;

	bool bWorkerEntityQueryInFlight;

	// Serialization and deserialization of the mapping.
	void WriteMappingToSchema(Schema_Object* Object) const;

	// The following methods are used to query the Runtime for all worker entities and update the mapping
	// based on the response.
	void QueryForServerWorkerEntities();
	void ServerWorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op);
	void ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op);
	void SendVirtualWorkerMappingUpdate() const;

	void AssignWorker(const PhysicalWorkerName& WorkerId, const Worker_EntityId& ServerWorkerEntityId);
};
