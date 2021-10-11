// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKTests/Public/GDKAutomationTestBase.h"
#include "Tests/TestDefinitions.h"

#include "Interop/Connection/SpatialConnectionManager.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialOutputDevice.h"

#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#endif // WITH_EDITOR

#include "CoreMinimal.h"
#include "Engine/Engine.h"

#define WORKERCONNECTION_TEST(TestName) GDK_AUTOMATION_TEST(Core, SpatialWorkerConnection, TestName)

using namespace SpatialGDK;

namespace
{
bool bClientConnectionProcessed = false;
bool bServerConnectionProcessed = false;
USpatialConnectionManager* ClientConnectionManager;
USpatialConnectionManager* ServerConnectionManager;
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupWorkerConnection, USpatialConnectionManager**, ConnectionManager, bool,
											   bConnectAsClient);
bool FSetupWorkerConnection::Update()
{
	(*ConnectionManager) = NewObject<USpatialConnectionManager>();
	const FURL TestURL = {};
	FString WorkerType = "AutomationWorker";

	(*ConnectionManager)->OnConnectedCallback.BindLambda([bConnectAsClient = this->bConnectAsClient]() {
		ConnectionProcessed(bConnectAsClient);
	});
	(*ConnectionManager)->OnFailedToConnectCallback.BindLambda(
		[bConnectAsClient = this->bConnectAsClient](uint8_t ErrorCode, const FString& ErrorMessage) {
			ConnectionProcessed(bConnectAsClient);
		});
	bool bUseReceptionist = false;
	StartSetupConnectionConfigFromURL(*ConnectionManager, TestURL, bUseReceptionist);
	FinishSetupConnectionConfig(*ConnectionManager, WorkerType, TestURL, bUseReceptionist);
	int32 PlayInEditorID = 0;
#if WITH_EDITOR
	(*ConnectionManager)->Connect(bConnectAsClient, PlayInEditorID);
#else
	(*ConnectionManager)->Connect(bConnectAsClient, 0);
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

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FCheckConnectionStatus, FAutomationTestBase*, Test, USpatialConnectionManager**,
												 ConnectionManager, bool, bExpectedIsConnected);
bool FCheckConnectionStatus::Update()
{
	Test->TestTrue(TEXT("Worker connection status is valid"), (*ConnectionManager)->IsConnected() == bExpectedIsConnected);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendReserveEntityIdsRequest, USpatialConnectionManager**, ConnectionManager);
bool FSendReserveEntityIdsRequest::Update()
{
	uint32_t NumOfEntities = 1;
	USpatialWorkerConnection* Connection = (*ConnectionManager)->GetWorkerConnection();
	if (Connection == nullptr)
	{
		return false;
	}

	Connection->SendReserveEntityIdsRequest(NumOfEntities, RETRY_UNTIL_COMPLETE);
	Connection->Flush();

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendCreateEntityRequest, USpatialConnectionManager**, ConnectionManager);
bool FSendCreateEntityRequest::Update()
{
	TArray<FWorkerComponentData> Components;
	const Worker_EntityId* EntityId = nullptr;
	USpatialWorkerConnection* Connection = (*ConnectionManager)->GetWorkerConnection();
	if (Connection == nullptr)
	{
		return false;
	}

	Connection->SendCreateEntityRequest(MoveTemp(Components), EntityId, RETRY_UNTIL_COMPLETE);
	Connection->Flush();

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSendDeleteEntityRequest, USpatialConnectionManager**, ConnectionManager);
bool FSendDeleteEntityRequest::Update()
{
	const Worker_EntityId EntityId = 0;
	USpatialWorkerConnection* Connection = (*ConnectionManager)->GetWorkerConnection();
	if (Connection == nullptr)
	{
		return false;
	}

	Connection->SendDeleteEntityRequest(EntityId, RETRY_UNTIL_COMPLETE);
	Connection->Flush();

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FFindWorkerResponseOfType, FAutomationTestBase*, Test, USpatialConnectionManager**,
												 ConnectionManager, uint8_t, ExpectedOpType);
bool FFindWorkerResponseOfType::Update()
{
	bool bFoundOpOfExpectedType = false;
	USpatialWorkerConnection* Connection = (*ConnectionManager)->GetWorkerConnection();
	if (Connection == nullptr)
	{
		return false;
	}

	Connection->Advance(0);
	for (const auto& Op : Connection->GetWorkerMessages())
	{
		if (Op.op_type == ExpectedOpType)
		{
			bFoundOpOfExpectedType = true;
			break;
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

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCleanupConnectionManager, USpatialConnectionManager**, ConnectionManager);
bool FCleanupConnectionManager::Update()
{
	(*ConnectionManager) = nullptr;

	return true;
}

#if WITH_EDITOR
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FChangeServerPort, uint16_t, NewServerPort);
bool FChangeServerPort::Update()
{
	GetMutableDefault<ULevelEditorPlaySettings>()->SetServerPort(NewServerPort);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCheckPortReset, FAutomationTestBase*, Test, uint16_t, PreviousServerPort);
bool FCheckPortReset::Update()
{
	uint16_t CurrentServerPort;
	GetDefault<ULevelEditorPlaySettings>()->GetServerPort(CurrentServerPort);
	Test->TestTrue(TEXT("Correctly reset the Server Port"), PreviousServerPort == CurrentServerPort);

	return true;
}
#endif // WITH_EDITOR

WORKERCONNECTION_TEST(GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = true;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ServerConnectionManager, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	return true;
}

#if WITH_EDITOR
WORKERCONNECTION_TEST(GIVEN_deployment_created_and_workers_connected_successfully_WHEN_changing_server_port_THEN_deployment_created_and_workers_connected_successfully)
{
	// GIVEN
	// This is basically an instance of the GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully test
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	bool bIsConnected = true;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ServerConnectionManager, bIsConnected));

	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	// WHEN
	// Cache the previous Server Port to be able to restore it at the end of the test
	uint16_t PreviousServerPort = 0;
	GetDefault<ULevelEditorPlaySettings>()->GetServerPort(PreviousServerPort);

	// Change the Server Port to a fixed value that is likely currently not in use
	const uint16_t NewServerPort = 54321;
	TestTrue(TEXT("New port value is not different from the current port value, change the current Server Port and try again!"), PreviousServerPort != NewServerPort);

	ADD_LATENT_AUTOMATION_COMMAND(FChangeServerPort(NewServerPort));

	// THEN
	// This is basically an instance of the GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully test
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));

	// Reset the Server Port as soon as possible to decrease the risk of the test tempering with the original value
	ADD_LATENT_AUTOMATION_COMMAND(FChangeServerPort(PreviousServerPort));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ServerConnectionManager, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	// Check if the Server Port was properly reset
	ADD_LATENT_AUTOMATION_COMMAND(FCheckPortReset(this, PreviousServerPort));

	return true;
}
#endif // WITH_EDITOR

WORKERCONNECTION_TEST(GIVEN_no_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connection_failed)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = false;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ClientConnectionManager, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, &ServerConnectionManager, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	GEngine->ForceGarbageCollection(true);

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_reserve_entity_ids_request_sent_THEN_reserve_entity_ids_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(&ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ClientConnectionManager, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ServerConnectionManager, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_create_entity_request_sent_THEN_create_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(&ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ServerConnectionManager, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ClientConnectionManager, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_delete_entity_request_sent_THEN_delete_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ClientConnectionManager, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(&ServerConnectionManager, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(&ServerConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ServerConnectionManager, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, &ClientConnectionManager, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ClientConnectionManager));
	ADD_LATENT_AUTOMATION_COMMAND(FCleanupConnectionManager(&ServerConnectionManager));

	return true;
}
