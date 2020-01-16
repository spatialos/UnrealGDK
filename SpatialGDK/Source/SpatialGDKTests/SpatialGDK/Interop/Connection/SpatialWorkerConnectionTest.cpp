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

void StartSetupConnectionConfigFromURL(USpatialWorkerConnection* Connection, const FURL& URL, bool& bOutUseReceptionist)
{
	bOutUseReceptionist = (URL.Host != SpatialConstants::LOCATOR_HOST) && !URL.HasOption(TEXT("locator"));
	if (bOutUseReceptionist)
	{
		Connection->ReceptionistConfig.SetReceptionistHost(URL.Host);
	}
	else
	{
		FLocatorConfig& LocatorConfig = Connection->LocatorConfig;
		LocatorConfig.PlayerIdentityToken = URL.GetOption(*SpatialConstants::URL_PLAYER_IDENTITY_OPTION, TEXT(""));
		LocatorConfig.LoginToken = URL.GetOption(*SpatialConstants::URL_LOGIN_OPTION, TEXT(""));
	}
}

void FinishSetupConnectionConfig(USpatialWorkerConnection* Connection, const FString& WorkerType, const FURL& URL, bool bUseReceptionist)
{
	// Finish setup for the config objects regardless of loading from command line or URL
	if (bUseReceptionist)
	{
		// Use Receptionist
		Connection->SetConnectionType(ESpatialConnectionType::Receptionist);

		FReceptionistConfig& ReceptionistConfig = Connection->ReceptionistConfig;
		ReceptionistConfig.WorkerType = WorkerType;

		const TCHAR* UseExternalIpForBridge = TEXT("useExternalIpForBridge");
		if (URL.HasOption(UseExternalIpForBridge))
		{
			FString UseExternalIpOption = URL.GetOption(UseExternalIpForBridge, TEXT(""));
			ReceptionistConfig.UseExternalIp = !UseExternalIpOption.Equals(TEXT("false"), ESearchCase::IgnoreCase);
		}
	}
	else
	{
		// Use Locator
		Connection->SetConnectionType(ESpatialConnectionType::Locator);
		FLocatorConfig& LocatorConfig = Connection->LocatorConfig;
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
	bool bUseReceptionist = false;
	StartSetupConnectionConfigFromURL(Connection, TestURL, bUseReceptionist);
	FinishSetupConnectionConfig(Connection, WorkerType, TestURL, bUseReceptionist);
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

DEFINE_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed);
bool FResetConnectionProcessed::Update()
{
	bClientConnectionProcessed = false;
	bServerConnectionProcessed = false;
	return true;
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
		Test->TestTrue(TEXT("Received Worker Repsonse of expected type"), bFoundOpOfExpectedType);
		return true;
	}
	else
	{
		return false;
	}
}

WORKERCONNECTION_TEST(GIVEN_running_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connected_successfully)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialWorkerConnection* ClientConnection = NewObject<USpatialWorkerConnection>();
	USpatialWorkerConnection* ServerConnection = NewObject<USpatialWorkerConnection>();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = true;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ClientConnection, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ServerConnection, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());

	return true;
}

WORKERCONNECTION_TEST(GIVEN_no_local_deployment_WHEN_connecting_client_and_server_worker_THEN_connection_failed)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));

	// WHEN
	USpatialWorkerConnection* ClientConnection = NewObject<USpatialWorkerConnection>();
	USpatialWorkerConnection* ServerConnection = NewObject<USpatialWorkerConnection>();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	bool bIsConnected = false;
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ClientConnection, bIsConnected));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckConnectionStatus(this, ServerConnection, bIsConnected));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_reserve_entity_ids_request_sent_THEN_reserve_entity_ids_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialWorkerConnection* ClientConnection = NewObject<USpatialWorkerConnection>();
	USpatialWorkerConnection* ServerConnection = NewObject<USpatialWorkerConnection>();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendReserveEntityIdsRequest(ServerConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnection, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnection, WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_create_entity_request_sent_THEN_create_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialWorkerConnection* ClientConnection = NewObject<USpatialWorkerConnection>();
	USpatialWorkerConnection* ServerConnection = NewObject<USpatialWorkerConnection>();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendCreateEntityRequest(ServerConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnection, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnection, WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());

	return true;
}

WORKERCONNECTION_TEST(GIVEN_valid_worker_connection_WHEN_delete_entity_request_sent_THEN_delete_entity_response_received)
{
	// GIVEN
	ADD_LATENT_AUTOMATION_COMMAND(FStartDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsRunning));

	// WHEN
	USpatialWorkerConnection* ClientConnection = NewObject<USpatialWorkerConnection>();
	USpatialWorkerConnection* ServerConnection = NewObject<USpatialWorkerConnection>();
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ClientConnection, true));
	ADD_LATENT_AUTOMATION_COMMAND(FSetupWorkerConnection(ServerConnection, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientAndServerWorkerConnection());

	// THEN
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(ClientConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FSendDeleteEntityRequest(ServerConnection));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ServerConnection, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));
	ADD_LATENT_AUTOMATION_COMMAND(FFindWorkerResponseOfType(this, ClientConnection, WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE));

	// CLEANUP
	ADD_LATENT_AUTOMATION_COMMAND(FStopDeployment());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForDeployment(this, EDeploymentState::IsNotRunning));
	ADD_LATENT_AUTOMATION_COMMAND(FResetConnectionProcessed());

	return true;
}

