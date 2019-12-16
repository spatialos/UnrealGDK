// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "SpatialWorkerConnection.h"

#include "CoreMinimal.h"

#define WORKERCONNECTION_TEST(TestName) \
	GDK_TEST(Core, USpatialWorkerConnection, TestName)

using namespace SpatialGDK;

namespace
{
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

DEFINE_LATENT_AUTOMATION_COMMAND(FSetupServerWorkerConnection);
bool FSetupServerWorkerConnection::Update()
{
	bool bConnectAsClient = false;

	const FURL TestURL = CreateTestURL();

	FString WorkerType = "UnrealWorker";
	USpatialWorkerConnection* Connection = NewObject<USpatialWorkerConnection>();
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

DEFINE_LATENT_AUTOMATION_COMMAND(FSetupClientWorkerConnection);
bool FSetupClientWorkerConnection::Update()
{
	bool bConnectAsClient = true;

	const FURL TestURL = CreateTestURL();

	FString WorkerType = "UnrealWorker";
	USpatialWorkerConnection* Connection = NewObject<USpatialWorkerConnection>();
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

DEFINE_LATENT_AUTOMATION_COMMAND(FWaitForClientWorkerConnection);
bool FWaitForClientWorkerConnection::Update()
{
	return false;
}

WORKERCONNECTION_TEST(GIVEN_WHEN_THEN)
{
	ADD_LATENT_AUTOMATION_COMMAND(FSetupServerWorkerConnection());
	ADD_LATENT_AUTOMATION_COMMAND(FSetupClientWorkerConnection());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForClientWorkerConnection());

	return true;
}
