// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "EngineClasses/SpatialNetConnection.h"
#include "SpatialCommonTypes.h"
#include "improbable/c_worker.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorkerEntitySystem, Log, All)

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;

class ClientConnectionManager
{
public:
	ClientConnectionManager(const FSubView& InSubView, USpatialNetDriver* InNetDriver);

	void Advance();

	void RegisterClientConnection(Worker_EntityId InWorkerEntityId, USpatialNetConnection* ClientConnection);
	void CleanUpClientConnection(USpatialNetConnection* ConnectionCleanedUp);

private:
	void EntityRemoved(const Worker_EntityId EntityId);

	void DisconnectPlayer(Worker_EntityId ClientEntityId);
	TWeakObjectPtr<USpatialNetConnection> FindClientConnectionFromWorkerEntityId(Worker_EntityId WorkerEntityId);
	static void CloseClientConnection(USpatialNetConnection* ClientConnection);

	const FSubView* SubView;
	USpatialNetDriver* NetDriver;

	TMap<Worker_EntityId_Key, FString> WorkerConnectionEntities;
	TMap<Worker_EntityId_Key, TWeakObjectPtr<USpatialNetConnection>> WorkerConnections;
};

} // namespace SpatialGDK
