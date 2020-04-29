// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"
#include "Interop/Connection/SpatialConnectionManager.h"
#include "CoreMinimal.h"

#define CONNECTIONMANAGER_TEST(TestName) \
	GDK_TEST(Core, SpatialConnectionManager, TestName)

class FTemporaryCommandLine
{
public:
    FTemporaryCommandLine(const FString& NewCommandLine)
    {
    	OldCommandLine = FCommandLine::GetOriginal();
    	FCommandLine::Set(*NewCommandLine);
    }

	~FTemporaryCommandLine()
    {
    	FCommandLine::Set(*OldCommandLine);
    }

private:
    FString OldCommandLine;
};

CONNECTIONMANAGER_TEST(SetupFromURL_Locator_CustomLocator)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("10.20.30.40?locator?customLocator?playeridentity=foo?login=bar"), TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
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
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("10.20.30.40?locator?playeridentity=foo?login=bar"), TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
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
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr,
		TEXT("10.20.30.40?devauth?customLocator?devauthtoken=foo?deployment=bar?playerid=666?displayname=n00bkilla?metadata=important"),
		TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
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
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("10.20.30.40?devauth"),TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 99.88.77.66");
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("LocatorHost", Manager->DevAuthConfig.LocatorHost, "99.88.77.66");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_Localhost)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("127.0.0.1"), TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("");
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, false);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_ExternalHost)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("10.20.30.40"), TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("");
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromURL_Receptionist_ExternalBridge)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	const FURL URL(nullptr, TEXT("127.0.0.1?useExternalIpForBridge"), TRAVEL_Absolute);
	FTemporaryCommandLine TemporaryCommandLine("");
	Manager->SetupConnectionConfigFromURL(URL, "SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Locator)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 10.20.30.40 -playerIdentityToken foo -loginToken bar");
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
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	FTemporaryCommandLine TemporaryCommandLine("-locatorHost 10.20.30.40 -devAuthToken foo -deployment bar -playerId 666 -displayName n00bkilla -metadata important");
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
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	FTemporaryCommandLine TemporaryCommandLine("-receptionistHost 10.20.30.40 -receptionistPort 666");
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.ReceptionistPort, 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_URL)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	FTemporaryCommandLine TemporaryCommandLine("10.20.30.40?someUnknownFlag?otherFlag -receptionistPort 666");
	const bool bSuccess = Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("Success", bSuccess, true);
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "10.20.30.40");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.ReceptionistPort, 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}

CONNECTIONMANAGER_TEST(SetupFromCommandLine_Receptionist_URLAndExternalBridge)
{
	USpatialConnectionManager* Manager = NewObject<USpatialConnectionManager>();

	// WHEN
	FTemporaryCommandLine TemporaryCommandLine("127.0.0.1?useExternalIpForBridge -receptionistPort 666");
	Manager->TrySetupConnectionConfigFromCommandLine("SomeWorkerType");

	// THEN
	TestEqual("UseExternalIp", Manager->ReceptionistConfig.UseExternalIp, true);
	TestEqual("ReceptionistHost", Manager->ReceptionistConfig.GetReceptionistHost(), "127.0.0.1");
	TestEqual("ReceptionistPort", Manager->ReceptionistConfig.ReceptionistPort, 666);
	TestEqual("WorkerType", Manager->ReceptionistConfig.WorkerType, "SomeWorkerType");

	return true;
}
