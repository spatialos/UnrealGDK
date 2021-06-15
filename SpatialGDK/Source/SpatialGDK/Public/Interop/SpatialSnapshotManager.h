// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/SchemaUtils.h"

#include "Interop/ReserveEntityIdsHandler.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "CoreMinimal.h"

#include "EntityQueryHandler.h"

class FGlobalStateManager;
class USpatialReceiver;
class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogSnapshotManager, Log, All)

DECLARE_DELEGATE(PostWorldWipeDelegate);

class SPATIALGDK_API SpatialSnapshotManager
{
public:
	SpatialSnapshotManager();

	void Init(USpatialWorkerConnection* InConnection, TSharedPtr<FGlobalStateManager> InGlobalStateManager);

	void WorldWipe(const PostWorldWipeDelegate& Delegate);
	void LoadSnapshot(const FString& SnapshotName);

	void Advance();

private:
	static void DeleteEntities(const Worker_EntityQueryResponseOp& Op, TWeakObjectPtr<USpatialWorkerConnection> Connection);

	TWeakObjectPtr<USpatialWorkerConnection> Connection;
	TWeakPtr<FGlobalStateManager> GlobalStateManager;
	SpatialGDK::ReserveEntityIdsHandler ReserveEntityIdsHandler;
	SpatialGDK::EntityQueryHandler QueryHandler;
};
