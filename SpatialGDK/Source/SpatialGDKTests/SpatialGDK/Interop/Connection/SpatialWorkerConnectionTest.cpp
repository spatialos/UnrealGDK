// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "SpatialWorkerConnection.h"

#include "SpatialGDKServices/LocalDeploymentManager/LocalDeploymentManagerUtilities.h"

#include "CoreMinimal.h"

#pragma optimize("", off)

#define WORKERCONNECTION_TEST(TestName) \
	GDK_TEST(Core, USpatialWorkerConnection, TestName)

using namespace SpatialGDK;

namespace
{
bool bClientConnectionProcessed = false;
bool bServerConnectionProcessed = false;

FURL CreateTestURL()
{
	FURL URL = {};
	URL.Protocol = L"unreal";
	URL.Port = 7777;
	URL.Valid = 1;
	URL.Map = L"/Game/Maps/UEDPIE_4_EmptyGym";

	return URL;
}

void StartSetupConnectionConfigFromURL(USpatialWorkerConnection* Connection, const FURL& URL, bool& bOutUseReceptionist)
{
	bOutUseReceptionist = !(URL.Host == SpatialConstants::LOCATOR_HOST || URL.HasOption(TEXT("locator")));
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
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSetupWorkerConnection, USpatialWorkerConnection*, Connection, bool, bConnectAsClient);
bool FSetupWorkerConnection::Update()
{
	const FURL TestURL = CreateTestURL();
	FString WorkerType = "UnrealWorker";

	Connection->OnConnectedCallback.BindLambda([bConnectAsClient = this->bConnectAsClient]()
	{
		//UE_LOG(LogTemp, Warning, TEXT("Worker connected successfully"));
		if (bConnectAsClient)
		{
			bClientConnectionProcessed = true;
		}
		else
		{
			bServerConnectionProcessed = true;
		}
	});
	Connection->OnFailedToConnectCallback.BindLambda([bConnectAsClient = this->bConnectAsClient](uint8_t ErrorCode, const FString& ErrorMessage)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Worker failed to connect: %d : %s"), ErrorCode, *ErrorMessage);
		if (bConnectAsClient)
		{
			bClientConnectionProcessed = true;
		}
		else
		{
			bServerConnectionProcessed = true;
		}
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
