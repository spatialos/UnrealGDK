// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/UnrealString.h"
#include "Internationalization/Regex.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"

#include <WorkerSDK/improbable/c_worker.h>

struct FConnectionConfig
{
	FConnectionConfig()
		: UseExternalIp(false)
		, EnableProtocolLoggingAtStartup(false)
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_KCP)
		, TcpMultiplexLevel(2) // This is a "finger-in-the-air" number.
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, TEXT("workerId"), WorkerId);
		FParse::Bool(CommandLine, TEXT("useExternalIpForBridge"), UseExternalIp);
		FParse::Bool(CommandLine, TEXT("enableProtocolLogging"), EnableProtocolLoggingAtStartup);
		FParse::Value(CommandLine, TEXT("protocolLoggingPrefix"), ProtocolLoggingPrefix);
        
#if PLATFORM_IOS || PLATFORM_ANDROID
		// On a mobile platform, you can only be a client worker, and therefore use the external IP.
		WorkerType = SpatialConstants::DefaultClientWorkerType.ToString();
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
		else if (!LinkProtocolString.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Unknown network protocol %s specified for connecting to SpatialOS. Defaulting to KCP."), *LinkProtocolString);
		}
	}

	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
	FString ProtocolLoggingPrefix;
	Worker_NetworkConnectionType LinkProtocol;
	Worker_ConnectionParameters ConnectionParams;
	uint8 TcpMultiplexLevel;
};

struct FReceptionistConfig : public FConnectionConfig
{
	FReceptionistConfig()
		: ReceptionistPort(SpatialConstants::DEFAULT_PORT)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		// Parse the commandline for receptionistHost, if it exists then use this as the host IP.
		if (!FParse::Value(CommandLine, TEXT("receptionistHost"), ReceptionistHost))
		{
			// If a receptionistHost is not specified then parse for an IP address as the first argument and use this instead.
			// This is how native Unreal handles connecting to other IPs, a map name can also be specified, in this case we use the default IP.
			FParse::Token(CommandLine, ReceptionistHost, 0);

			FRegexPattern Ipv4RegexPattern(TEXT("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"));

			FRegexMatcher IpV4RegexMatcher(Ipv4RegexPattern, *ReceptionistHost);
			if (!IpV4RegexMatcher.FindNext())
			{
				// If an IP is not specified then use default.
				ReceptionistHost = GetDefault<USpatialGDKSettings>()->DefaultReceptionistHost;
				if (ReceptionistHost.Compare(SpatialConstants::LOCAL_HOST) != 0)
				{
					UseExternalIp = true;
				}
			}
		}

		FParse::Value(CommandLine, TEXT("receptionistPort"), ReceptionistPort);
	}

	FString ReceptionistHost;
	uint16 ReceptionistPort;
};

struct FLocatorConfig : public FConnectionConfig
{
	FLocatorConfig()
		: LocatorHost(SpatialConstants::LOCATOR_HOST) {
		const TCHAR* CommandLine = FCommandLine::Get();
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
		FParse::Value(CommandLine, TEXT("playerIdentityToken"), PlayerIdentityToken);
		FParse::Value(CommandLine, TEXT("loginToken"), LoginToken);
	}

	FString LocatorHost;
	FString PlayerIdentityToken;
	FString LoginToken;
};
