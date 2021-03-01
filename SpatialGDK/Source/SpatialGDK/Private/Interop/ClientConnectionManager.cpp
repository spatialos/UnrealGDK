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

	for (const Worker_Op& Op : *SubViewDelta.WorkerMessages)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE)
		{
			const Worker_CommandResponseOp& CommandResponseOp = Op.op.command_response;
			if (CommandResponseOp.response.component_id == SpatialConstants::WORKER_COMPONENT_ID)
			{
				if (CommandResponseOp.response.command_index == SpatialConstants::WORKER_DISCONNECT_COMMAND_ID)
				{
					const Worker_RequestId RequestId = CommandResponseOp.request_id;
					const Worker_EntityId ClientEntityId = DisconnectRequestToConnectionEntityId.FindAndRemoveChecked(RequestId);

					if (Op.op.command_response.status_code == WORKER_STATUS_CODE_SUCCESS)
					{
						TWeakObjectPtr<USpatialNetConnection> ClientConnection = FindClientConnectionFromWorkerEntityId(ClientEntityId);
						if (ClientConnection.IsValid())
						{
							ClientConnection->CleanUp();
						};
					}
					else
					{
						UE_LOG(LogSpatialReceiver, Error, TEXT("SystemEntityCommand failed: request id: %d, message: %s"), RequestId,
							   UTF8_TO_TCHAR(CommandResponseOp.message));
					}
				}
			}
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
	const Worker_RequestId RequestId = NetDriver->Connection->SendCommandRequest(ClientEntityId, &Request, RETRY_UNTIL_COMPLETE, {});

	DisconnectRequestToConnectionEntityId.Add(RequestId, ClientEntityId);
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
