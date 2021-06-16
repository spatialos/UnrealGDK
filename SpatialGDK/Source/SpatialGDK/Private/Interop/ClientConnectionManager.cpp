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

DEFINE_LOG_CATEGORY(LogWorkerEntitySystem);

namespace SpatialGDK
{
struct FSubViewDelta;

ClientConnectionManager::ClientConnectionManager(const FSubView& InSubView, USpatialNetDriver* InNetDriver)
	: SubView(&InSubView)
	, NetDriver(InNetDriver)
{
	ResponseHandler.AddResponseHandler(SpatialConstants::WORKER_COMPONENT_ID, SpatialConstants::WORKER_DISCONNECT_COMMAND_ID,
									   FOnCommandResponseWithOp::FDelegate::CreateRaw(this, &ClientConnectionManager::OnRequestReceived));
}

void ClientConnectionManager::Advance()
{
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

	ResponseHandler.ProcessOps(*SubViewDelta.WorkerMessages);
}

void ClientConnectionManager::OnRequestReceived(const Worker_Op&, const Worker_CommandResponseOp& CommandResponseOp)
{
	if (CommandResponseOp.response.component_id == SpatialConstants::WORKER_COMPONENT_ID)
	{
		if (CommandResponseOp.response.command_index == SpatialConstants::WORKER_DISCONNECT_COMMAND_ID)
		{
			const Worker_RequestId RequestId = CommandResponseOp.request_id;
			FSpatialEntityId ClientEntityId;
			if (DisconnectRequestToConnectionEntityId.RemoveAndCopyValue(RequestId, ClientEntityId))
			{
				if (CommandResponseOp.status_code == WORKER_STATUS_CODE_SUCCESS)
				{
					TWeakObjectPtr<USpatialNetConnection> ClientConnection = FindClientConnectionFromWorkerEntityId(ClientEntityId);
					if (ClientConnection.IsValid())
					{
						ClientConnection->CleanUp();
					};
				}
				else
				{
					UE_LOG(LogWorkerEntitySystem, Error, TEXT("SystemEntityCommand failed: request id: %d, message: %s"), RequestId,
						   UTF8_TO_TCHAR(CommandResponseOp.message));
				}
			}
		}
	}
}

void ClientConnectionManager::RegisterClientConnection(const FSpatialEntityId InWorkerEntityId, USpatialNetConnection* ClientConnection)
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

void ClientConnectionManager::DisconnectPlayer(FSpatialEntityId ClientEntityId)
{
	Worker_CommandRequest Request = {};
	Request.component_id = SpatialConstants::WORKER_COMPONENT_ID;
	Request.command_index = SpatialConstants::WORKER_DISCONNECT_COMMAND_ID;
	Request.schema_type = Schema_CreateCommandRequest();

	const Worker_RequestId RequestId = NetDriver->Connection->SendCommandRequest(ClientEntityId, &Request, RETRY_UNTIL_COMPLETE, {});

	DisconnectRequestToConnectionEntityId.Add(RequestId, ClientEntityId);
}

void ClientConnectionManager::EntityRemoved(const FSpatialEntityId EntityId)
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

TWeakObjectPtr<USpatialNetConnection> ClientConnectionManager::FindClientConnectionFromWorkerEntityId(const FSpatialEntityId WorkerEntityId)
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
