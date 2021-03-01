// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "EngineClasses/SpatialNetConnection.h"
#include "SpatialCommonTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorkerEntitySystem, Log, All)

class USpatialNetDriver;

namespace SpatialGDK
{
class FSubView;

// The client connection manager is responsible for maintaining the current set of client connections this
// server worker knows about. This is maintained as a system entity ID to connection mapping, where the system entity
// represents the client corresponding to the connection.
//
// It is informed of changes in client connections (such as a client connecting) and uses a subview with the System
// component as a tag in order to tell when system entities have been deleted in order to clean up the corresponding
// connection.
class ClientConnectionManager
{
public:
	ClientConnectionManager(const FSubView& InSubView, USpatialNetDriver* InNetDriver);

	void Advance();

	void RegisterClientConnection(Worker_EntityId InWorkerEntityId, USpatialNetConnection* ClientConnection);
	void CleanUpClientConnection(USpatialNetConnection* ConnectionCleanedUp);

	void DisconnectPlayer(Worker_EntityId ClientEntityId);

private:
	void EntityRemoved(const Worker_EntityId EntityId);

	TWeakObjectPtr<USpatialNetConnection> FindClientConnectionFromWorkerEntityId(Worker_EntityId WorkerEntityId);
	static void CloseClientConnection(USpatialNetConnection* ClientConnection);

	const FSubView* SubView;
	USpatialNetDriver* NetDriver;

	TMap<Worker_EntityId_Key, TWeakObjectPtr<USpatialNetConnection>> WorkerConnections;

	TMap<Worker_RequestId_Key, Worker_EntityId> DisconnectRequestToConnectionEntityId;
};

} // namespace SpatialGDK
