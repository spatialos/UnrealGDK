#include <improbable/c_worker.h>
#include "UnrealString.h"

struct ConnectionConfig
{
	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
	Worker_NetworkConnectionType LinkProtocol;
};

struct ReceptionistConfig : public ConnectionConfig
{
	FString ReceptionistHost;
	uint8 ReceptionistPort;
};

struct LocatorConfig : public ConnectionConfig
{
	FString LocatorHost;
	Worker_LocatorParameters LocatorParameters;
};
