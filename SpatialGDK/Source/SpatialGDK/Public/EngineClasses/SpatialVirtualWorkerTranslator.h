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
class ASpatialVirtualWorkerTranslator : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DELEGATE_OneParam(FOnWorkerAssignmentChanged, const TArray<FString>& /*Assignment*/);

	void Init(USpatialNetDriver* InNetDriver);

	void AuthorityChanged(const Worker_AuthorityChangeOp& AuthChangeOp);

	void OnComponentAdded(const Worker_AddComponentOp& Op);
	void OnComponentUpdated(const Worker_ComponentUpdateOp& Op);

	void UpdateEntityAclWriteForEntity(Worker_EntityId EntityId);

	virtual void BeginPlay() override;

	const TArray<ZoneId>& GetZones() const { return Zones; }
	const TArray<VirtualWorkerId>& GetVirtualWorkers() const { return VirtualWorkers; }
	// TODO - VWId/FString discrepancy
	const TArray<FString>& GetVirtualWorkerAssignments() const { return VirtualWorkerAssignment; }

	FOnWorkerAssignmentChanged OnWorkerAssignmentChanged;

private:

	UFUNCTION()
	void OnRep_VirtualWorkerAssignment();

	void OnWorkerComponentReceived(const Worker_ComponentData& Data);

	void AssignWorker(const FString& WorkerId);

	// this will be static information
	// this map is simulating the info that will be passed in somehow from the editor
	TArray<ZoneId> Zones;
	TArray<VirtualWorkerId> VirtualWorkers;

	TQueue<VirtualWorkerId> UnassignedVirtualWorkers;

	UPROPERTY(ReplicatedUsing = OnRep_VirtualWorkerAssignment)
	TArray<FString> VirtualWorkerAssignment;

	USpatialNetDriver* NetDriver;
};
