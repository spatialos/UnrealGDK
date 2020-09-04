// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOutputDevice.h"
#include "SpatialGDKTests/SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

#include "CoreMinimal.h"

#define WORKERCONNECTION_TEST(TestName) GDK_TEST(Core, SpatialWorkerConnection, TestName)

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

void StartSetupConnectionConfigFromURL(USpatialConnectionManager* ConnectionManager, const FURL& URL, bool& bOutUseReceptionist)
{
	bOutUseReceptionist = (URL.Host != SpatialConstants::LOCATOR_HOST) && !URL.HasOption(TEXT("locator"));
	if (bOutUseReceptionist)
	{
		ConnectionManager->ReceptionistConfig.SetupFromURL(URL);
	}
	else
	{
		FLocatorConfig& LocatorConfig = ConnectionManager->LocatorConfig;
		LocatorConfig.PlayerIdentityToken = URL.GetOption(*SpatialConstants::URL_PLAYER_IDENTITY_OPTION, TEXT(""));
		LocatorConfig.LoginToken = URL.GetOption(*SpatialConstants::URL_LOGIN_OPTION, TEXT(""));
	}
}

void FinishSetupConnectionConfig(USpatialConnectionManager* ConnectionManager, const FString& WorkerType, const FURL& URL,
								 bool bUseReceptionist)
{
	// Finish setup for the config objects regardless of loading from command line or URL
	if (bUseReceptionist)
	{
		// Use Receptionist
		ConnectionManager->SetConnectionType(ESpatialConnectionType::Receptionist);

		FReceptionistConfig& ReceptionistConfig = ConnectionManager->ReceptionistConfig;
		ReceptionistConfig.WorkerType = WorkerType;
	}
	else
	{
		// Use Locator
		ConnectionManager->SetConnectionType(ESpatialConnectionType::Locator);
		FLocatorConfig& LocatorConfig = ConnectionManager->LocatorConfig;
		FParse::Value(FCommandLine::Get(), TEXT("locatorHost"), LocatorConfig.LocatorHost);
		LocatorConfig.WorkerType = WorkerType;
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupWorkerConnection, USpatialConnectionManager*, ConnectionManager, bool,
											   bConnectAsClient);
bool FSetupWorkerConnection::Update()
{
	const FURL TestURL = {};
	FString WorkerType = "AutomationWorker";

	ConnectionManager->OnConnectedCallback.BindLambda([bConnectAsClient = this->bConnectAsClient]() {
		ConnectionProcessed(bConnectAsClient);
	});
	ConnectionManager->OnFailedToConnectCallback.BindLambda(
		[bConnectAsClient = this->bConnectAsClient](uint8_t ErrorCode, const FString& ErrorMessage) {
			ConnectionProcessed(bConnectAsClient);
		});
	bool bUseReceptionist = false;
	StartSetupConnectionConfigFromURL(ConnectionManager, TestURL, bUseReceptionist);
	FinishSetupConnectionConfig(ConnectionManager, WorkerType, TestURL, bUseReceptionist);
	int32 PlayInEditorID = 0;
#if WITH_EDITOR
	ConnectionManager->Connect(bConnectAsClient, PlayInEditorID);
#else
	ConnectionManager->Connect(bConnectAsClient, 0);
#endif
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection);
bool FWaitForClientAndServerWorkerConnection::Update()
{
	return bClientConnectionProcessed && bServerConnectionProcessed;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed);
bool FResetConnectionProcessed::Update()
{
	bClientConnectionProcessed = false;
	bServerConnectionProcessed = false;
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckConnectionStatus, FAutomationTestBase*, Test, USpatialConnectionManager*,
												 ConnectionManager, bool, bExpectedIsConnected);
bool FCheckConnectionStatus::Update()
{
	Test->TestTrue(TEXT("Worker connection status is valid"), ConnectionManager->IsConnected() == bExpectedIsConnected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendReserveEntityIdsRequest, USpatialConnectionManager*, ConnectionManager);
bool FSendReserveEntityIdsRequest::Update()
{
	uint32_t NumOfEntities = 1;
	USpatialWorkerConnection* Connection = ConnectionManager->GetWorkerConnection();
	Connection->SendReserveEntityIdsRequest(NumOfEntities);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendCreateEntityRequest, USpatialConnectionManager*, ConnectionManager);
bool FSendCreateEntityRequest::Update()
{
	TArray<FWorkerComponentData> Components;
	const Worker_EntityId* EntityId = nullptr;
	USpatialWorkerConnection* Connection = ConnectionManager->GetWorkerConnection();
	Connection->SendCreateEntityRequest(MoveTemp(Components), EntityId);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendDeleteEntityRequest, USpatialConnectionManager*, ConnectionManager);
bool FSendDeleteEntityRequest::Update()
{
	const Worker_EntityId EntityId = 0;
	USpatialWorkerConnection* Connection = ConnectionManager->GetWorkerConnection();
	Connection->SendDeleteEntityRequest(EntityId);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FFindWorkerResponseOfType, FAutomationTestBase*, Test, USpatialConnectionManager*,
												 ConnectionManager, uint8_t, ExpectedOpType);
bool FFindWorkerResponseOfType::Update()
{
	bool bFoundOpOfExpectedType = false;
	USpatialWorkerConnection* Connection = ConnectionManager->GetWorkerConnection();
	for (const auto& Ops : Connection->GetOpList())
	{
		for (uint32_t i = 0; i < Ops.Count; i++)
		{
			if (Ops.Ops[i].op_type == ExpectedOpType)
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
		Test->TestTrue(TEXT("Received Worker Repsonse of expected type"), bFoundOpOfExpectedType);
		return true;
	}
	else
	{
		return false;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCleanupConnectionManager, USpatialConnectionManager*, ConnectionManager);
bool FCleanupConnectionManager::Update()
{
	ConnectionManager->RemoveFromRoot();

	return true;
}

WORKERCONNECTION_TEST(GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialConnectionManager* ClientConnectionManager = NewObject<USpatialConnectionManager>();
	USpatialConnectionManager* ServerConnectionManager = NewObject<USpatialConnectionManager>();
	ClientConnectionManager->AddToRoot();
	ServerConnectionManager->AddToRoot();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = true;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ServerConnectionManager, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ServerConnectionManager));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_no_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connection_failed)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));

	// WHEN
	USpatialConnectionManager* ClientConnectionManager = NewObject<USpatialConnectionManager>();
	USpatialConnectionManager* ServerConnectionManager = NewObject<USpatialConnectionManager>();
	ClientConnectionManager->AddToRoot();
	ServerConnectionManager->AddToRoot();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = false;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ServerConnectionManager, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ServerConnectionManager));

	GEngine->ForceGarbageCollection(true);

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_reserve_entity_ids_request_sent_THEN_reserve_entity_ids_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialConnectionManager* ClientConnectionManager = NewObject<USpatialConnectionManager>();
	USpatialConnectionManager* ServerConnectionManager = NewObject<USpatialConnectionManager>();
	ClientConnectionManager->AddToRoot();
	ServerConnectionManager->AddToRoot();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnectionManager, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnectionManager, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ServerConnectionManager));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_create_entity_request_sent_THEN_create_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialConnectionManager* ClientConnectionManager = NewObject<USpatialConnectionManager>();
	USpatialConnectionManager* ServerConnectionManager = NewObject<USpatialConnectionManager>();
	ClientConnectionManager->AddToRoot();
	ServerConnectionManager->AddToRoot();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnectionManager, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnectionManager, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ServerConnectionManager));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_delete_entity_request_sent_THEN_delete_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialConnectionManager* ClientConnectionManager = NewObject<USpatialConnectionManager>();
	USpatialConnectionManager* ServerConnectionManager = NewObject<USpatialConnectionManager>();
	ClientConnectionManager->AddToRoot();
	ServerConnectionManager->AddToRoot();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnectionManager, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnectionManager, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(ServerConnectionManager));

	return true;
}
