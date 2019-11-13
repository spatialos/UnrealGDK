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
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_MODULAR_UDP)
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
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_UDP;
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

class FLocatorConfig : public FConnectionConfig
{
public:
	FLocatorConfig()
	{
		LoadDefaults();
	}

	void LoadDefaults()
	{
		UseExternalIp = true;
		LocatorHost = SpatialConstants::LOCATOR_HOST;
	}

	bool TryLoadCommandLineArgs()
	{
		bool bSuccess = true;
		const TCHAR* CommandLine = FCommandLine::Get();
		bSuccess &= FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
		bSuccess &= FParse::Value(CommandLine, TEXT("playerIdentityToken"), PlayerIdentityToken);
		bSuccess &= FParse::Value(CommandLine, TEXT("loginToken"), LoginToken);
		return bSuccess;
	}

	FString LocatorHost;
	FString PlayerIdentityToken;
	FString LoginToken;
};

class FReceptionistConfig : public FConnectionConfig
{
public:
	FReceptionistConfig()
	{
		LoadDefaults();
	}

	void LoadDefaults()
	{
		ReceptionistPort = SpatialConstants::DEFAULT_PORT;
		SetReceptionistHost(GetDefault<USpatialGDKSettings>()->DefaultReceptionistHost);
	}

	bool TryLoadCommandLineArgs()
	{
		bool bSuccess = true;
		const TCHAR* CommandLine = FCommandLine::Get();

		// Parse the command line for receptionistHost, if it exists then use this as the host IP.
		if (!FParse::Value(CommandLine, TEXT("receptionistHost"), ReceptionistHost))
		{
			// If a receptionistHost is not specified then parse for an IP address as the first argument and use this instead.
			// This is how native Unreal handles connecting to other IPs, a map name can also be specified, in this case we use the default IP.
			FString URLAddress;
			FParse::Token(CommandLine, URLAddress, 0);
			FRegexPattern Ipv4RegexPattern(TEXT("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"));
			FRegexMatcher IpV4RegexMatcher(Ipv4RegexPattern, *URLAddress);
			bSuccess = IpV4RegexMatcher.FindNext();
			if (bSuccess)
			{
				SetReceptionistHost(URLAddress);
			}
		}

		FParse::Value(CommandLine, TEXT("receptionistPort"), ReceptionistPort);
		return bSuccess;
	}

	void SetReceptionistHost(const FString& host)
	{
		ReceptionistHost = host;
		UseExternalIp = ReceptionistHost.Compare(SpatialConstants::LOCAL_HOST) != 0;
	}

	FString GetReceptionistHost() const { return ReceptionistHost; }

	uint16 ReceptionistPort;

private:
	FString ReceptionistHost;

};
