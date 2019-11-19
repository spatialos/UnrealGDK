// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <WorkerSDK/improbable/c_schema.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class USpatialNetDriver;

typedef FString PhysicalWorkerName;

class SPATIALGDK_API SpatialVirtualWorkerTranslator
{
public:
	SpatialVirtualWorkerTranslator();

	void Init(USpatialNetDriver* InNetDriver);

	// Returns true if the Translator has received the information needed to map virtual workers to physical workers.
	// Currently that is only the number of virtual workers desired.
	bool IsReady() const { return bIsReady; }

	void SetDesiredVirtualWorkerCount(uint32 NumberOfVirtualWorkers);

	// Returns the name of the worker currently assigned to VirtualWorkerId id or nullptr if there is
	// no worker assigned.
	// TODO(harkness): Do we want to copy this data? Otherwise it's only guaranteed to be valid until
	// the next mapping update.
	const FString* GetPhysicalWorkerForVirtualWorker(VirtualWorkerId id);

	// On receiving a version of the translation state, apply that to the internal mapping.
	void ApplyVirtualWorkerManagerData(Schema_Object* ComponentObject);

	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);

	// Authority may change on one of two components we care about:
	// 1) The translation component, in which case this worker is now authoritative on the virtual to physical worker translation.
	// 2) The ACL component for some entity, in which case this worker is now authoritative for the entity and will be
	// responsible for updating the ACL in the future if this worker loses authority.
	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);

private:
	USpatialNetDriver* NetDriver;

	TMap<VirtualWorkerId, PhysicalWorkerName>  VirtualToPhysicalWorkerMapping;
	TQueue<VirtualWorkerId> UnassignedVirtualWorkers;

	bool bWorkerEntityQueryInFlight;

	bool bIsReady;

	// The WorkerId of this worker, for logging purposes.
	FString WorkerId;

	// Serialization and deserialization of the mapping.
	void ApplyMappingFromSchema(Schema_Object* Object);
	void WriteMappingToSchema(Schema_Object* Object);

	// The following methods are used to query the Runtime for all worker entities and update the mapping
	// based on the response.
	void QueryForWorkerEntities();
	void WorkerEntityQueryDelegate(const Worker_EntityQueryResponseOp& Op);
	void ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op);
	void SendVirtualWorkerMappingUpdate();

	void AssignWorker(const FString& WorkerId);
};
