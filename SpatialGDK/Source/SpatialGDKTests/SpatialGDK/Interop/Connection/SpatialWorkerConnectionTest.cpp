// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOutputDevice.h"
#include "SpatialGDKTests/SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

#include "CoreMinimal.h"

#define WORKERCONNECTION_TEST(TestName) \
	GDK_TEST(Core, SpatialWorkerConnection, TestName)

using namespace SpatialGDK;

namespace
{
bool bClientConnectionProcessed = false;
bool bServerConnectionProcessed = false;
const double MAX_WAIT_TIME = 10.0;

void ConnectionProcessed(bool bConnectAsClient)
{
	if (bConnectAsClient)
	{
		bClientConnectionProcessed = true;
	}
	else
	{
		bServerConnectionProcessed = true;
	}
}
} // anonymous namespace

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForSeconds, double, Seconds);
bool FWaitForSeconds::Update()
{
	const double NewTime = FPlatformTime::Seconds();

	if (NewTime - StartTime >= Seconds)
	{
		return true;
	}
	else
	{
		return false;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupWorkerConnection, USpatialWorkerConnection*, Connection, bool, bConnectAsClient);
bool FSetupWorkerConnection::Update()
{
	const FURL TestURL = {};
	FString WorkerType = "AutomationWorker";

	Connection->OnConnectedCallback.BindLambda([bConnectAsClient = this->bConnectAsClient]()
	{
		ConnectionProcessed(bConnectAsClient);
	});
	Connection->OnFailedToConnectCallback.BindLambda([bConnectAsClient = this->bConnectAsClient](uint8_t ErrorCode, const FString& ErrorMessage)
	{
		ConnectionProcessed(bConnectAsClient);
	});

	Connection->SetupConnectionConfigFromURL(TestURL, WorkerType);

	int32 PlayInEditorID = 0;
#if WITH_EDITOR
	Connection->Connect(bConnectAsClient, PlayInEditorID);
#else
	Connection->Connect(bConnectAsClient, 0);
#endif
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection);
bool FWaitForClientAndServerWorkerConnection::Update()
{
	return bClientConnectionProcessed && bServerConnectionProcessed;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckConnectionStatus, FAutomationTestBase*, Test, USpatialWorkerConnection*, Connection, bool, bExpectedIsConnected);
bool FCheckConnectionStatus::Update()
{
	Test->TestTrue(TEXT("Worker connection status is valid"), Connection->IsConnected() == bExpectedIsConnected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendReserveEntityIdsRequest, USpatialWorkerConnection*, Connection);
bool FSendReserveEntityIdsRequest::Update()
{
	uint32_t NumOfEntities = 1;
	Connection->SendReserveEntityIdsRequest(NumOfEntities);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendCreateEntityRequest, USpatialWorkerConnection*, Connection);
bool FSendCreateEntityRequest::Update()
{
	TArray<Worker_ComponentData> Components;
	const Worker_EntityId* EntityId = nullptr;
	Connection->SendCreateEntityRequest(MoveTemp(Components), EntityId);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendDeleteEntityRequest, USpatialWorkerConnection*, Connection);
bool FSendDeleteEntityRequest::Update()
{
	const Worker_EntityId EntityId = 0;
	Connection->SendDeleteEntityRequest(EntityId);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendCommandRequest, USpatialWorkerConnection*, Connection);
bool FSendCommandRequest::Update()
{
	const Worker_EntityId EntityId = 0;
	Worker_CommandRequest CommandRequest = {};
	uint32_t CommandId = 0;
	Connection->SendCommandRequest(EntityId, &CommandRequest, CommandId);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FFindWorkerResponseOfType, FAutomationTestBase*, Test, USpatialWorkerConnection*, Connection, uint8_t, ExpectedOpType);
bool FFindWorkerResponseOfType::Update()
{
	bool bFoundOpOfExpectedType = false;
	for (const auto& OpList : Connection->GetOpList())
	{
		for (uint32_t i = 0; i < OpList->op_count; i++)
		{
			if (OpList->ops[i].op_type == ExpectedOpType)
			{
				bFoundOpOfExpectedType = true;
				break;
			}
		}
	}

	bool bReachedTimeout = false;
	const double NewTime = FPlatformTime::Seconds();
	if (NewTime - StartTime >= MAX_WAIT_TIME)
	{
		bReachedTimeout = true;
	}

	if (bFoundOpOfExpectedType || bReachedTimeout)
	{
		Test->TestTrue(TEXT("Received Worker Response of expected type"), bFoundOpOfExpectedType);
		return true;
	}
	else
	{
		return false;
	}
}

struct FConnectionsFixture;
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupConnectionFixture, USpatialWorkerConnection*, ServerConnection, USpatialWorkerConnection*, ClientConnection);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCleanupConnectionFixture, USpatialWorkerConnection*, ServerConnection, USpatialWorkerConnection*, ClientConnection);

struct FConnectionsFixture
{
	FConnectionsFixture()
		: ClientConnection(NewObject<USpatialWorkerConnection>())
		, ServerConnection(NewObject<USpatialWorkerConnection>())
	{
		
		ADD_LATENT_AUTOMATION_COMMAND(FSetupConnectionFixture(ServerConnection, ClientConnection));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
		ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());
	}

	~FConnectionsFixture()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionFixture(ServerConnection, ClientConnection));
	}

	USpatialWorkerConnection* ClientConnection = nullptr;
	USpatialWorkerConnection* ServerConnection = nullptr;
};

bool FSetupConnectionFixture::Update()
{
	ClientConnection->AddToRoot();
	ServerConnection->AddToRoot();

	return true;
}

bool FCleanupConnectionFixture::Update()
{
	bClientConnectionProcessed = false;
	bServerConnectionProcessed = false;
	ClientConnection->RemoveFromRoot();
	ServerConnection->RemoveFromRoot();

	return true;
}

struct FDeploymentFixture
{
	FDeploymentFixture(FAutomationTestBase* InTest)
		: Test(InTest)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(Test, EDeploymentState::IsRunning));
	}

	~FDeploymentFixture()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(Test, EDeploymentState::IsNotRunning));
	}

	FAutomationTestBase* Test = nullptr;
};

WORKERCONNECTION_TEST(GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN

	// THEN
	bool bIsConnected = true;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, Connections.ClientConnection, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, Connections.ServerConnection, bIsConnected));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_no_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connection_failed)
{
	// GIVEN
	FConnectionsFixture Connections;

	// WHEN

	// THEN
	bool bIsConnected = false;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, Connections.ClientConnection, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, Connections.ServerConnection, bIsConnected));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_reserve_entity_ids_request_sent_THEN_reserve_entity_ids_response_received)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(Connections.ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(Connections.ServerConnection));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ServerConnection, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ClientConnection, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_create_entity_request_sent_THEN_create_entity_response_received)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(Connections.ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(Connections.ServerConnection));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ServerConnection, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ClientConnection, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_delete_entity_request_sent_THEN_delete_entity_response_received)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(Connections.ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(Connections.ServerConnection));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ServerConnection, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ClientConnection, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_command_request_sent_THEN_command_request_response_received)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendCommandRequest(Connections.ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendCommandRequest(Connections.ServerConnection));

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ServerConnection, WORKER_OP_TYPE_COMMAND_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ClientConnection, WORKER_OP_TYPE_COMMAND_RESPONSE));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_doing_nothing_THEN_metrics_received)
{
	// GIVEN
	FDeploymentFixture Deployment(this);
	FConnectionsFixture Connections;

	// WHEN

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ServerConnection, WORKER_OP_TYPE_METRICS));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, Connections.ClientConnection, WORKER_OP_TYPE_METRICS));

	return true;
}
