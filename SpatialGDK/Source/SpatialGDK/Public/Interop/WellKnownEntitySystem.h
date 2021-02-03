// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Connection/SpatialWorkerConnection.h"
#include "EngineClasses/SpatialVirtualWorkerTranslationManager.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "GlobalStateManager.h"
#include "SpatialView/SubView.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWellKnownEntitySystem, Log, All)

namespace SpatialGDK
{
class WellKnownEntitySystem
{
public:
	WellKnownEntitySystem(const FSubView& SubView, USpatialNetDriver* InNetDriver, USpatialWorkerConnection* InConnection,
						  int InNumberOfWorkers, SpatialVirtualWorkerTranslator& InVirtualWorkerTranslator,
						  UGlobalStateManager& InGlobalStateManager);
	void Advance();

private:
	void ProcessComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update);
	void ProcessComponentAdd(const Worker_ComponentId ComponentId, Schema_ComponentData* Data);
	void ProcessAuthorityGain(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId);
	void ProcessEntityAdd(const Worker_EntityId EntityId);

	void InitializeVirtualWorkerTranslationManager();
	void MaybeClaimSnapshotPartition();
	void CreateServerWorkerEntity();
	void RetryServerWorkerEntityCreation(Worker_EntityId EntityId, int AttemptCounter);
	void SendClaimPartitionRequest(Worker_EntityId SystemWorkerEntityId, Worker_PartitionId PartitionId) const;
	void UpdatePartitionEntityInterestAndPosition();

	const FSubView* SubView;

	USpatialNetDriver* NetDriver;

	TUniquePtr<SpatialVirtualWorkerTranslationManager> VirtualWorkerTranslationManager;
	SpatialVirtualWorkerTranslator* VirtualWorkerTranslator;
	UGlobalStateManager* GlobalStateManager;
	USpatialWorkerConnection* Connection;
	int NumberOfWorkers;
};

} // namespace SpatialGDK
