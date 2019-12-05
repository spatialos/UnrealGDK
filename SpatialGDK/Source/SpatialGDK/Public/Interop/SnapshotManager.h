// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

class UGlobalStateManager;
class USpatialReceiver;

DECLARE_LOG_CATEGORY_EXTERN(LogSnapshotManager, Log, All)

class SPATIALGDK_API SpatialSnapshotManager
{
public:
	SpatialSnapshotManager();

	void Init(USpatialWorkerConnection* InConnection, UGlobalStateManager* InGlobalStateManager, USpatialReceiver* InReceiver);

	void WorldWipe(const PostWorldWipeDelegate& Delegate);
	void DeleteEntities(const Worker_EntityQueryResponseOp& Op);
	void LoadSnapshot(const FString& SnapshotName);

private:
	TWeakObjectPtr<USpatialWorkerConnection> Connection;
	TWeakObjectPtr<UGlobalStateManager> GlobalStateManager;
	TWeakObjectPtr<USpatialReceiver> Receiver;
};
