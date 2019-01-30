// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_worker.h>

struct FConnectionConfig
{
	FConnectionConfig()
		: UseExternalIp(false)
		, EnableProtocolLoggingAtStartup(false)
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_KCP)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, TEXT("workerType"), WorkerType);
		FParse::Value(CommandLine, TEXT("workerId"), WorkerId);
		FParse::Bool(CommandLine, TEXT("useExternalIpForBridge"), UseExternalIp);
		FParse::Bool(CommandLine, TEXT("enableProtocolLogging"), EnableProtocolLoggingAtStartup);
		FParse::Value(CommandLine, TEXT("protocolLoggingPrefix"), ProtocolLoggingPrefix);
        
#if PLATFORM_IOS || PLATFORM_ANDROID
		// On a mobile platform, you can only be a client worker, and therefore use the external IP.
		WorkerType = SpatialConstants::ClientWorkerType;
		UseExternalIp = true;
#endif

		FString LinkProtocolString;
		FParse::Value(CommandLine, TEXT("linkProtocol"), LinkProtocolString);
		if (LinkProtocolString == TEXT("Tcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_TCP;
		}
		else if (LinkProtocolString == TEXT("Kcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_KCP;
		}
#if !(PLATFORM_PS4 || PLATFORM_XBOXONE)
		// RakNet is not compiled for console platforms.
		else if (LinkProtocolString == TEXT("RakNet"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_RAKNET;
		}
#endif // !(PLATFORM_PS4 || PLATFORM_XBOXONE)
	}

	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
	FString ProtocolLoggingPrefix;
	Worker_NetworkConnectionType LinkProtocol;
	Worker_ConnectionParameters ConnectionParams;
};

struct FReceptionistConfig : public FConnectionConfig
{
	FReceptionistConfig()
		: ReceptionistHost(SpatialConstants::LOCAL_HOST)
		, ReceptionistPort(SpatialConstants::DEFAULT_PORT)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, TEXT("receptionistHost"), ReceptionistHost);
		FParse::Value(CommandLine, TEXT("receptionistPort"), ReceptionistPort);
	}

	FString ReceptionistHost;
	uint16 ReceptionistPort;
};

struct FLegacyLocatorConfig : public FConnectionConfig
{
	FLegacyLocatorConfig()
		: LocatorHost(TEXT("locator.improbable.io"))
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, TEXT("projectName"), ProjectName);
		FParse::Value(CommandLine, TEXT("deploymentName"), DeploymentName);
		FParse::Value(CommandLine, TEXT("loginToken"), LoginToken);
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
	}

	FString ProjectName;
	FString DeploymentName;
	FString LocatorHost;
	FString LoginToken;
};

struct FLocatorConfig : public FConnectionConfig
{
	FLocatorConfig()
		: LocatorHost(TEXT("locator.improbable.io")) {
		const TCHAR* CommandLine = FCommandLine::Get();
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
		FParse::Value(CommandLine, TEXT("playerIdentityToken"), PlayerIdentityToken);
		FParse::Value(CommandLine, TEXT("loginToken"), LoginToken);
	}

	FString LocatorHost;
	FString PlayerIdentityToken;
	FString LoginToken;
};
