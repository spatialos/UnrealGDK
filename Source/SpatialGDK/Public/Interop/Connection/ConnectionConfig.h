// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"

#include <improbable/c_worker.h>

struct ConnectionConfig
{
	ConnectionConfig()
		: UseExternalIp(false)
		, EnableProtocolLoggingAtStartup(false)
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_RAKNET)
	{
		const TCHAR* commandLine = FCommandLine::Get();

		FParse::Value(commandLine, *FString("workerType"), WorkerType);
		FParse::Value(commandLine, *FString("workerId"), WorkerId);
		FParse::Bool(commandLine, *FString("useExternalIpForBridge"), UseExternalIp);

		auto LinkProtocolString = FString("");
		FParse::Value(commandLine, *FString("linkProtocol"), LinkProtocolString);
		LinkProtocol = LinkProtocolString == "Tcp" ? WORKER_NETWORK_CONNECTION_TYPE_TCP : WORKER_NETWORK_CONNECTION_TYPE_RAKNET;
	}

	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
	Worker_NetworkConnectionType LinkProtocol;
};

struct ReceptionistConfig : public ConnectionConfig
{
	ReceptionistConfig()
		: ReceptionistHost(TEXT("127.0.0.1"))
		, ReceptionistPort(7777)
	{
		const TCHAR* commandLine = FCommandLine::Get();

		FParse::Value(commandLine, *FString("receptionistHost"), ReceptionistHost);
		FParse::Value(commandLine, *FString("receptionistPort"), ReceptionistPort);
	}

	FString ReceptionistHost;
	uint16 ReceptionistPort;
};

struct LocatorConfig : public ConnectionConfig
{
	LocatorConfig()
		: LocatorHost(TEXT("locator.improbable.io"))
	{
		const TCHAR* commandLine = FCommandLine::Get();

		FParse::Value(commandLine, *FString("projectName"), LoginToken);
		FParse::Value(commandLine, *FString("loginToken"), LoginToken);
		FParse::Value(commandLine, *FString("locatorHost"), LocatorHost);
	}

	FString ProjectName;
	FString LocatorHost;
	FString LoginToken;
};
