// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"

#include <improbable/c_worker.h>

struct FConnectionConfig
{
	FConnectionConfig()
		: UseExternalIp(false)
		, EnableProtocolLoggingAtStartup(false)
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_RAKNET)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, *FString(TEXT("workerType")), WorkerType);
		FParse::Value(CommandLine, *FString(TEXT("workerId")), WorkerId);
		FParse::Bool(CommandLine, *FString(TEXT("useExternalIpForBridge")), UseExternalIp);

		FString LinkProtocolString;
		FParse::Value(CommandLine, *FString(TEXT("linkProtocol")), LinkProtocolString);
		LinkProtocol = LinkProtocolString == TEXT("Tcp") ? WORKER_NETWORK_CONNECTION_TYPE_TCP : WORKER_NETWORK_CONNECTION_TYPE_RAKNET;
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
		: ReceptionistHost(TEXT("127.0.0.1"))
		, ReceptionistPort(7777)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, *FString("receptionistHost"), ReceptionistHost);
		FParse::Value(CommandLine, *FString("receptionistPort"), ReceptionistPort);
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

		FParse::Value(CommandLine, *FString("projectName"), ProjectName);
		FParse::Value(CommandLine, *FString("loginToken"), LoginToken);
		FParse::Value(CommandLine, *FString("locatorHost"), LocatorHost);
	}

	FString ProjectName;
	FString LocatorHost;
	FString LoginToken;
};
