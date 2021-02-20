// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ClientConnectionManager.h"

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/PlayerController.h"
#include "Improbable/SpatialEngineConstants.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "Interop/SpatialReceiver.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"
#include "SpatialGDKLLM.h"

DEFINE_LOG_CATEGORY(LogWorkerEntitySystem);

namespace SpatialGDK
{
struct FSubViewDelta;

ClientConnectionManager::ClientConnectionManager(const FSubView& InSubView, USpatialNetDriver* InNetDriver)
	: SubView(&InSubView)
	, NetDriver(InNetDriver)
{
}

void ClientConnectionManager::Advance()
{
	LLM_PLATFORM_SCOPE_SPATIAL(ELLMTagSpatialGDK::ClientConnectionManager);
	const FSubViewDelta& SubViewDelta = SubView->GetViewDelta();
	for (const EntityDelta& Delta : SubViewDelta.EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::REMOVE:
			EntityRemoved(Delta.EntityId);
		default:
			break;
		}
	}
}

void ClientConnectionManager::RegisterClientConnection(const Worker_EntityId InWorkerEntityId, USpatialNetConnection* ClientConnection)
{
	WorkerConnections.Add(InWorkerEntityId, ClientConnection);
}

void ClientConnectionManager::CleanUpClientConnection(USpatialNetConnection* ConnectionCleanedUp)
{
	if (ConnectionCleanedUp->ConnectionClientWorkerSystemEntityId != SpatialConstants::INVALID_ENTITY_ID)
	{
		WorkerConnections.Remove(ConnectionCleanedUp->ConnectionClientWorkerSystemEntityId);
	}
}

void ClientConnectionManager::DisconnectPlayer(Worker_EntityId ClientEntityId)
{
	Worker_CommandRequest Request = {};
	Request.component_id = SpatialConstants::WORKER_COMPONENT_ID;
	Request.command_index = SpatialConstants::WORKER_DISCONNECT_COMMAND_ID;
	Request.schema_type = Schema_CreateCommandRequest();
	Worker_RequestId RequestId = NetDriver->Connection->SendCommandRequest(ClientEntityId, &Request, RETRY_UNTIL_COMPLETE, {});

	SystemEntityCommandDelegate CommandResponseDelegate;
	CommandResponseDelegate.BindLambda([this, ClientEntityId](const Worker_CommandResponseOp&) {
		TWeakObjectPtr<USpatialNetConnection> ClientConnection = FindClientConnectionFromWorkerEntityId(ClientEntityId);
		if (ClientConnection.IsValid())
		{
			ClientConnection->CleanUp();
		}
	});

	NetDriver->Receiver->AddSystemEntityCommandDelegate(RequestId, CommandResponseDelegate);
}

void ClientConnectionManager::EntityRemoved(const Worker_EntityId EntityId)
{
	// Check to see if we are removing a system entity for a client worker connection. If so clean up the
	// ClientConnection to delete any and all actors for this connection's controller.
	TWeakObjectPtr<USpatialNetConnection> ClientConnectionPtr;
	if (WorkerConnections.RemoveAndCopyValue(EntityId, ClientConnectionPtr))
	{
		if (USpatialNetConnection* ClientConnection = ClientConnectionPtr.Get())
		{
			CloseClientConnection(ClientConnection);
		}
	}
}

TWeakObjectPtr<USpatialNetConnection> ClientConnectionManager::FindClientConnectionFromWorkerEntityId(const Worker_EntityId WorkerEntityId)
{
	if (TWeakObjectPtr<USpatialNetConnection>* ClientConnectionPtr = WorkerConnections.Find(WorkerEntityId))
	{
		return *ClientConnectionPtr;
	}

	return {};
}

void ClientConnectionManager::CloseClientConnection(USpatialNetConnection* ClientConnection)
{
	ClientConnection->CleanUp();
}

} // Namespace SpatialGDK
