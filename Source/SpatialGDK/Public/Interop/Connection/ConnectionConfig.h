#pragma once

#include <improbable/c_worker.h>
#include "UnrealString.h"

struct ConnectionConfig
{
	ConnectionConfig()
		: UseExternalIp(false)
		, EnableProtocolLoggingAtStartup(false)
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_RAKNET)
	{}

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
	{}

	FString ReceptionistHost;
	uint16 ReceptionistPort;
};

struct LocatorConfig : public ConnectionConfig
{
	FString LocatorHost;
	Worker_LocatorParameters LocatorParameters;
};
