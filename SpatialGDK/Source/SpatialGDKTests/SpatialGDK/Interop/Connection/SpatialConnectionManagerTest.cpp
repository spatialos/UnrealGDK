// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "Tests/TestDefinitions.h"

#define CONNECTIONMANAGER_TEST(TestName) GDK_TEST(Core, SpatialConnectionManager, TestName)

class FTemporaryCommandLine
{
public:
	explicit FTemporaryCommandLine(const FString& NewCommandLine)
	{
		if (OldCommandLine.IsEmpty())
		{
			OldCommandLine = FCommandLine::GetOriginal();
			FCommandLine::Set(*NewCommandLine);
			bDidSetCommandLine = true;
		}
	}

	~FTemporaryCommandLine()
	{
		if (bDidSetCommandLine)
		{
			FCommandLine::Set(*OldCommandLine);
			OldCommandLine.Empty();
		}
	}

private:
	static FString OldCommandLine;
	bool bDidSetCommandLine = false;
};

FString FTemporaryCommandLine::OldCommandLine;

CONNECTIONMANAGER_TEST(SetupFromURL_Locator_CustomLocator)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
	const FURL URL(nullptr, TEXT("10.20.30.40?locator?customLocator?playeridentity=foo?login=bar"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("LocatorHost", Manager->LocatorConfig.LocatorHost, "10.20.30.40");
	TestEqual("PlayerIdentityToken", Manager->LocatorConfig.PlayerIdentityToken, "foo");
	TestEqual("LoginToken", Manager->LocatorConfig.LoginToken, "bar");
	TestEqual("WorkerType", Manager->LocatorConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Locator_LocatorHost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
	const FURL URL(nullptr, TEXT("10.20.30.40?locator?playeridentity=foo?login=bar"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("LocatorHost", Manager->LocatorConfig.LocatorHost, "99.88.77.66");
	TestEqual("PlayerIdentityToken", Manager->LocatorConfig.PlayerIdentityToken, "foo");
	TestEqual("LoginToken", Manager->LocatorConfig.LoginToken, "bar");
	TestEqual("WorkerType", Manager->LocatorConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_DevAuth)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
	const FURL URL(
		nullptr,
		TEXT("10.20.30.40?devauth?customLocator?devauthtoken=foo?deployment=bar?playerid=666?displayname=n00bkilla?metadata=important"),
		TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("LocatorHost", Manager->DevAuthConfig.LocatorHost, "10.20.30.40");
	TestEqual("DevAuthToken", Manager->DevAuthConfig.DevelopmentAuthToken, "foo");
	TestEqual("Deployment", Manager->DevAuthConfig.Deployment, "bar");
	TestEqual("PlayerId", Manager->DevAuthConfig.PlayerId, "666");
	TestEqual("DisplayName", Manager->DevAuthConfig.DisplayName, "n00bkilla");
	TestEqual("Metadata", Manager->DevAuthConfig.MetaData, "important");
	TestEqual("WorkerType", Manager->DevAuthConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_DevAuth_LocatorHost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
	const FURL URL(nullptr, TEXT("10.20.30.40?devauth"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("LocatorHost", Manager->DevAuthConfig.LocatorHost, "99.88.77.66");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_Localhost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("");
	const FURL URL(nullptr, TEXT("127.0.0.1:777"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 777);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_ExternalHost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("");
	const FURL URL(nullptr, TEXT("10.20.30.40:777"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 777);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_ExternalBridge)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("");
	const FURL URL(nullptr, TEXT("127.0.0.1:777?useExternalIpForBridge"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 777);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_ExternalBridgeNoHost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("");
	const FURL URL(nullptr, TEXT("?useExternalIpForBridge"), TRAVEL_Absolute);
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), SpatialConstants::DEFAULT_PORT);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Locator)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 10.20.30.40 -playerIdentityToken foo -loginToken bar");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("LocatorHost", Manager->LocatorConfig.LocatorHost, "10.20.30.40");
	TestEqual("PlayerIdentityToken", Manager->LocatorConfig.PlayerIdentityToken, "foo");
	TestEqual("LoginToken", Manager->LocatorConfig.LoginToken, "bar");
	TestEqual("WorkerType", Manager->LocatorConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_DevAuth)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine(
		"-locatorHost 10.20.30.40 -devAuthToken foo -deployment bar -playerId 666 -displayName n00bkilla -metadata important");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("LocatorHost", Manager->DevAuthConfig.LocatorHost, "10.20.30.40");
	TestEqual("DevAuthToken", Manager->DevAuthConfig.DevelopmentAuthToken, "foo");
	TestEqual("Deployment", Manager->DevAuthConfig.Deployment, "bar");
	TestEqual("PlayerId", Manager->DevAuthConfig.PlayerId, "666");
	TestEqual("DisplayName", Manager->DevAuthConfig.DisplayName, "n00bkilla");
	TestEqual("Metadata", Manager->DevAuthConfig.MetaData, "important");
	TestEqual("WorkerType", Manager->DevAuthConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_ReceptionistHost)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-receptionistHost 10.20.30.40 -receptionistPort 666");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_ReceptionistHostLocal)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-receptionistPort 666");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_ReceptionistHostLocalExternalBridge)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("-receptionistPort 666 -useExternalIpForBridge true");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_URL)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("10.20.30.40?someUnknownFlag?otherFlag -receptionistPort 666");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_URLAndExternalBridge)
{
	// GIVEN
	FTemporaryCommandLine TemporaryCommandLine("127.0.0.1?useExternalIpForBridge -receptionistPort 666");
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.GetReceptionistPort(), 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}
