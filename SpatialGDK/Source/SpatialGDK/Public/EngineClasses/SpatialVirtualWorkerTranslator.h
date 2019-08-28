// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialVirtualWorkerTranslator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialVirtualWorkerTranslator, Log, All)

class USpatialNetDriver;

typedef FString ZoneId;
typedef FString VirtualWorkerId;
typedef FString WorkerId;

UCLASS(SpatialType = (Singleton, ServerOnly))
class USpatialVirtualWorkerTranslator : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorkerAssignmentChanged, const TArray<FString>& /*Assignment*/);

	void Init(USpatialNetDriver* InNetDriver);

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);

	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);
	void OnComponentRemoved(const Worker_RemoveComponentOp& Op);

	void Tick();

	const TArray<ZoneId>& GetZones() const { return Zones; }
	const TArray<VirtualWorkerId>& GetVirtualWorkers() const { return VirtualWorkers; }
	// TODO - VWId/FString discrepancy
	const TArray<FString>& GetVirtualWorkerAssignments() const { return VirtualWorkerAssignment; }

	void ApplyVirtualWorkerManagerData(const Worker_ComponentData& Data);
	void ApplyVirtualWorkerManagerUpdate(const Worker_ComponentUpdate& Data);

	mutable FOnWorkerAssignmentChanged OnWorkerAssignmentChanged;

	void QueueAclAssignmentRequest(const Worker_EntityId EntityId);

private:

	UFUNCTION()

	void AssignWorker(const FString& WorkerId);
	void UnassignWorker(const FString& WorkerId);

	void ProcessQueuedAclAssignmentRequests();

	void SetAclWriteAuthority(const Worker_EntityId EntityId, const FString& WorkerId);
	void QueryForWorkerEntities();

	void ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op);

	void SendVirtualWorkerMappingUpdate();

	// this will be static information
	// this map is simulating the info that will be passed in somehow from the editor
	TArray<ZoneId> Zones;
	TArray<VirtualWorkerId> VirtualWorkers;

	TQueue<VirtualWorkerId> UnassignedVirtualWorkers;

	TArray<FString> VirtualWorkerAssignment;

	TArray<Worker_EntityId> AclWriteAuthAssignmentRequests;

	USpatialNetDriver* NetDriver;

	Worker_EntityId TranslatorEntityId;

	bool bWorkerEntityQueryInFlight;
};
