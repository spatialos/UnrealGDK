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
		, LinkProtocol(WORKER_NETWORK_CONNECTION_TYPE_MODULAR_KCP)
		, TcpMultiplexLevel(2) // This is a "finger-in-the-air" number.
		// These settings will be overridden by Spatial GDK settings before connection applied (see PreConnectInit)
		, TcpNoDelay(0)
		, UdpUpstreamIntervalMS(0)
		, UdpDownstreamIntervalMS(0)
	{
		const TCHAR* CommandLine = FCommandLine::Get();

		FParse::Value(CommandLine, TEXT("workerId"), WorkerId);
		FParse::Bool(CommandLine, *SpatialConstants::URL_USE_EXTERNAL_IP_FOR_BRIDGE_OPTION, UseExternalIp);
		FParse::Bool(CommandLine, TEXT("enableProtocolLogging"), EnableProtocolLoggingAtStartup);
		FParse::Value(CommandLine, TEXT("protocolLoggingPrefix"), ProtocolLoggingPrefix);

		FString LinkProtocolString;
		FParse::Value(CommandLine, TEXT("linkProtocol"), LinkProtocolString);
		if (LinkProtocolString == TEXT("Tcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_TCP;
		}
		else if (LinkProtocolString == TEXT("Kcp"))
		{
			LinkProtocol = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_KCP;
		}
		else if (!LinkProtocolString.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Unknown network protocol %s specified for connecting to SpatialOS. Defaulting to KCP."), *LinkProtocolString);
		}
	}

	void PreConnectInit(const bool bConnectAsClient)
	{
		const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

		if (WorkerType.IsEmpty())
		{
			WorkerType = bConnectAsClient ? SpatialConstants::DefaultClientWorkerType.ToString() : SpatialConstants::DefaultServerWorkerType.ToString();
			UE_LOG(LogTemp, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *WorkerType);
		}

		if (WorkerId.IsEmpty())
		{
			WorkerId = WorkerType + FGuid::NewGuid().ToString();
		}

		TcpNoDelay = (SpatialGDKSettings->bTcpNoDelay ? 1 : 0);

		UdpUpstreamIntervalMS = 255; // This is set unreasonably large but is flushed at the rate of USpatialGDKSettings::OpsUpdateRate.
		UdpDownstreamIntervalMS = (bConnectAsClient ? SpatialGDKSettings->UdpClientDownstreamUpdateIntervalMS : SpatialGDKSettings->UdpServerDownstreamUpdateIntervalMS);
	}

	FString WorkerId;
	FString WorkerType;
	bool UseExternalIp;
	bool EnableProtocolLoggingAtStartup;
	FString ProtocolLoggingPrefix;
	Worker_NetworkConnectionType LinkProtocol;
	Worker_ConnectionParameters ConnectionParams = {};
	uint8 TcpMultiplexLevel;
	uint8 TcpNoDelay;
	uint8 UdpUpstreamIntervalMS;
	uint8 UdpDownstreamIntervalMS;
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

		if (GetDefault<USpatialGDKSettings>()->IsRunningInChina())
		{
			LocatorHost = SpatialConstants::LOCATOR_HOST_CN;
		}
		else
		{
			LocatorHost = SpatialConstants::LOCATOR_HOST;
		}
	}

	bool TryLoadCommandLineArgs()
	{
		bool bSuccess = true;
		const TCHAR* CommandLine = FCommandLine::Get();
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
		bSuccess &= FParse::Value(CommandLine, TEXT("playerIdentityToken"), PlayerIdentityToken);
		bSuccess &= FParse::Value(CommandLine, TEXT("loginToken"), LoginToken);
		return bSuccess;
	}

	FString LocatorHost;
	FString PlayerIdentityToken;
	FString LoginToken;
};

class FDevAuthConfig : public FLocatorConfig
{
public:
	FDevAuthConfig()
	{
		LoadDefaults();
	}

	void LoadDefaults()
	{
		UseExternalIp = true;
		PlayerId = SpatialConstants::DEVELOPMENT_AUTH_PLAYER_ID;

		if (GetDefault<USpatialGDKSettings>()->IsRunningInChina())
		{
			LocatorHost = SpatialConstants::LOCATOR_HOST_CN;
		}
		else
		{
			LocatorHost = SpatialConstants::LOCATOR_HOST;
		}
	}

	bool TryLoadCommandLineArgs()
	{
		const TCHAR* CommandLine = FCommandLine::Get();
		FParse::Value(CommandLine, TEXT("locatorHost"), LocatorHost);
		FParse::Value(CommandLine, TEXT("deployment"), Deployment);
		FParse::Value(CommandLine, TEXT("playerId"), PlayerId);
		FParse::Value(CommandLine, TEXT("displayName"), DisplayName);
		FParse::Value(CommandLine, TEXT("metaData"), MetaData);
		const bool bSuccess = FParse::Value(CommandLine, TEXT("devAuthToken"), DevelopmentAuthToken);
		return bSuccess;
	}

	FString DevelopmentAuthToken;
	FString Deployment;
	FString PlayerId;
	FString DisplayName;
	FString MetaData;
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
		const TCHAR* CommandLine = FCommandLine::Get();

		// Get command line options first since the URL handling will modify the CommandLine string
		FParse::Value(CommandLine, TEXT("receptionistPort"), ReceptionistPort);

		// Parse the command line for receptionistHost, if it exists then use this as the host IP.
		FString Host;
		if (!FParse::Value(CommandLine, TEXT("receptionistHost"), Host))
		{
			// If a receptionistHost is not specified then parse for an IP address as the first argument and use this instead.
			// This is how native Unreal handles connecting to other IPs, a map name can also be specified, in this case we use the default IP.
			FString URLAddress;
			FParse::Token(CommandLine, URLAddress, false /* UseEscape */);
			const FURL URL(nullptr /* Base */, *URLAddress, TRAVEL_Absolute);
			if (URL.Valid)
			{
				SetupFromURL(URL);
			}
		}
		else
		{
			SetReceptionistHost(Host);
		}

		return true;
	}

	void SetupFromURL(const FURL& URL)
	{
		if (!URL.Host.IsEmpty())
		{
			SetReceptionistHost(URL.Host);
		}
		if (URL.HasOption(*SpatialConstants::URL_USE_EXTERNAL_IP_FOR_BRIDGE_OPTION))
		{
			UseExternalIp = true;
		}
	}

	FString GetReceptionistHost() const { return ReceptionistHost; }

	uint16 ReceptionistPort;

private:
	void SetReceptionistHost(const FString& Host)
	{
		if (!Host.IsEmpty())
		{
			ReceptionistHost = Host;
		}
	}

	FString ReceptionistHost;
};
