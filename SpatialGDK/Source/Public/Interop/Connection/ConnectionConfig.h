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

		FString LinkProtocolString;
		FParse::Value(CommandLine, TEXT("linkProtocol"), LinkProtocolString);
		if (LinkProtocolString == TEXT("Tcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_TCP;
		}
		if (LinkProtocolString == TEXT("RakNet"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_RAKNET;
		}
		if (LinkProtocolString == TEXT("Kcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_KCP;
		}

#if PLATFORM_PS4 == 1 || PLATFORM_XBOXONE == 1
		// Use KCP in all cases on console.
		LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_KCP;
#endif
	}

	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
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

struct FLocatorConfig : public FConnectionConfig
{
	FLocatorConfig()
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

struct FLocatorV2Config : public FConnectionConfig
{
	FLocatorV2Config()
		: LocatorHost(TEXT("locator.improbable.io")) {
		const TCHAR* CommandLine = FCommandLine::Get();
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
	}

	FString LocatorHost;
	FString PlayerIdentityToken;
	FString LoginToken;
};
