// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

class UGlobalStateManager;
class USpatialReceiver;
class UWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSnapshotManager, Log, All)

DECLARE_DELEGATE(PostWorldWipeDelegate);

class SPATIALGDK_API SpatialSnapshotManager
{
public:
	SpatialSnapshotManager();

	void Init(UWorkerConnection* InConnection, UGlobalStateManager* InGlobalStateManager, USpatialReceiver* InReceiver);

	void WorldWipe(const PostWorldWipeDelegate& Delegate);
	void LoadSnapshot(const FString& SnapshotName);

private:
	static void DeleteEntities(const Worker_EntityQueryResponseOp& Op, TWeakObjectPtr<UWorkerConnection> Connection);

	TWeakObjectPtr<UWorkerConnection> Connection;
	TWeakObjectPtr<UGlobalStateManager> GlobalStateManager;
	TWeakObjectPtr<USpatialReceiver> Receiver;
};
