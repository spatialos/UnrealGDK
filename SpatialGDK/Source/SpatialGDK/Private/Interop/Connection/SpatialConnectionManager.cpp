// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialConnectionManager.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/ErrorCodeRemapping.h"

#include "Async/Async.h"
#include "Improbable/SpatialEngineConstants.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSpatialConnectionManager);

using namespace SpatialGDK;

struct ConfigureConnection
{
	ConfigureConnection(const FConnectionConfig& InConfig, const bool bConnectAsClient)
		: Config(InConfig)
		, Params()
		, WorkerType(*Config.WorkerType)
		, WorkerSDKLogFilePrefix(*FormatWorkerSDKLogFilePrefix())
	{
		Params = Worker_DefaultConnectionParameters();

		Params.worker_type = WorkerType.Get();

		Logsink.logsink_type = WORKER_LOGSINK_TYPE_ROTATING_FILE;
		Logsink.rotating_logfile_parameters.log_prefix = WorkerSDKLogFilePrefix.Get();
		Logsink.rotating_logfile_parameters.max_log_files = 1;
		// TODO: When upgrading to Worker SDK 14.6.2, remove the WorkerSDKLogFileSize parameter and set this to 0 for infinite file size
		Logsink.rotating_logfile_parameters.max_log_file_size_bytes = Config.WorkerSDKLogFileSize;

		uint32_t Categories = 0;
		if (Config.EnableWorkerSDKOpLogging)
		{
			Categories |= WORKER_LOG_CATEGORY_API;
		}
		if (Config.EnableWorkerSDKProtocolLogging)
		{
			Categories |= WORKER_LOG_CATEGORY_NETWORK_STATUS | WORKER_LOG_CATEGORY_NETWORK_TRAFFIC;
		}
		Logsink.filter_parameters.categories = Categories;
		Logsink.filter_parameters.level = Config.WorkerSDKLogLevel;

		Params.logsinks = &Logsink;
		Params.logsink_count = 1;
		Params.enable_logging_at_startup = Categories != 0;

		Params.component_vtable_count = 0;
		Params.default_component_vtable = &DefaultVtable;

		Params.network.connection_type = Config.LinkProtocol;
		Params.network.use_external_ip = Config.UseExternalIp;
		Params.network.tcp.multiplex_level = Config.TcpMultiplexLevel;
		if (Config.TcpNoDelay)
		{
			Params.network.tcp.downstream_tcp.flush_delay_millis = 0;
			Params.network.tcp.upstream_tcp.flush_delay_millis = 0;
		}

		// We want the bridge to worker messages to be compressed; not the worker to bridge messages.
		Params.network.kcp.upstream_compression = nullptr;
		Params.network.kcp.downstream_compression = &EnableCompressionParams;

		Params.network.kcp.upstream_kcp.flush_interval_millis = Config.UdpUpstreamIntervalMS;
		Params.network.kcp.downstream_kcp.flush_interval_millis = Config.UdpDownstreamIntervalMS;

#if WITH_EDITOR
		Params.network.tcp.downstream_heartbeat = &HeartbeatParams;
		Params.network.tcp.upstream_heartbeat = &HeartbeatParams;
		Params.network.kcp.downstream_heartbeat = &HeartbeatParams;
		Params.network.kcp.upstream_heartbeat = &HeartbeatParams;
#endif

		// Use insecure connections default.
		Params.network.kcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;
		Params.network.tcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;

		// Override the security type to be secure only if the user has requested it and we are not using an editor build.
		if ((!bConnectAsClient && GetDefault<USpatialGDKSettings>()->bUseSecureServerConnection)
			|| (bConnectAsClient && GetDefault<USpatialGDKSettings>()->bUseSecureClientConnection))
		{
#if WITH_EDITOR
			UE_LOG(LogSpatialWorkerConnection, Warning,
				   TEXT("Secure connection requested but this is not supported in Editor builds. Connection will be insecure."));
#else
			Params.network.kcp.security_type = WORKER_NETWORK_SECURITY_TYPE_TLS;
			Params.network.tcp.security_type = WORKER_NETWORK_SECURITY_TYPE_TLS;
#endif
		}
	}

	FString FormatWorkerSDKLogFilePrefix() const
	{
		FString FinalLogFilePrefix = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir());
		if (!Config.WorkerSDKLogPrefix.IsEmpty())
		{
			FinalLogFilePrefix += Config.WorkerSDKLogPrefix;
		}
		FinalLogFilePrefix += Config.WorkerId + TEXT("-");
		return FinalLogFilePrefix;
	}

	const FConnectionConfig& Config;
	Worker_ConnectionParameters Params;
	FTCHARToUTF8 WorkerType;
	FTCHARToUTF8 WorkerSDKLogFilePrefix;
	Worker_ComponentVtable DefaultVtable{};
	Worker_CompressionParameters EnableCompressionParams{};
	Worker_LogsinkParameters Logsink{};

#if WITH_EDITOR
	Worker_HeartbeatParameters HeartbeatParams{ WORKER_DEFAULTS_HEARTBEAT_INTERVAL_MILLIS, MAX_int64 };
#endif
};

void USpatialConnectionManager::FinishDestroy()
{
	UE_LOG(LogSpatialConnectionManager, Log, TEXT("Destroying SpatialConnectionManager."));

	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialConnectionManager::DestroyConnection()
{
	if (WorkerLocator)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerLocator = WorkerLocator] {
			Worker_Locator_Destroy(WorkerLocator);
		});

		WorkerLocator = nullptr;
	}

	if (WorkerConnection != nullptr)
	{
		WorkerConnection->DestroyConnection();
		WorkerConnection = nullptr;
	}

	bIsConnected = false;
}

void USpatialConnectionManager::Connect(bool bInitAsClient, uint32 PlayInEditorID)
{
	if (bIsConnected)
	{
		check(bInitAsClient == bConnectAsClient);
		AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<USpatialConnectionManager>(this)] {
			if (WeakThis.IsValid())
			{
				WeakThis->OnConnectionSuccess();
			}
			else
			{
				UE_LOG(LogSpatialConnectionManager, Error, TEXT("SpatialConnectionManager is not valid but was already connected."));
			}
		});
		return;
	}

	bConnectAsClient = bInitAsClient;

	const ISpatialGDKEditorModule* SpatialGDKEditorModule = FModuleManager::GetModulePtr<ISpatialGDKEditorModule>("SpatialGDKEditor");
	if (SpatialGDKEditorModule != nullptr && SpatialGDKEditorModule->ShouldConnectToCloudDeployment())
	{
		if (bInitAsClient)
		{
			DevAuthConfig.Deployment = SpatialGDKEditorModule->GetSpatialOSCloudDeploymentName();
			DevAuthConfig.WorkerType = SpatialConstants::DefaultClientWorkerType.ToString();
			DevAuthConfig.UseExternalIp = true;
			StartDevelopmentAuth(SpatialGDKEditorModule->GetDevAuthToken());
			return;
		}
		else if (SpatialGDKEditorModule->ShouldConnectServerToCloud())
		{
			ReceptionistConfig.UseExternalIp = true;
		}
	}

	switch (GetConnectionType())
	{
	case ESpatialConnectionType::Receptionist:
		ConnectToReceptionist(PlayInEditorID);
		break;
	case ESpatialConnectionType::Locator:
		ConnectToLocator(&LocatorConfig);
		break;
	case ESpatialConnectionType::DevAuthFlow:
		StartDevelopmentAuth(DevAuthConfig.DevelopmentAuthToken);
		break;
	}
}

void USpatialConnectionManager::OnLoginTokens(void* UserData, const Worker_LoginTokensResponse* LoginTokens)
{
	if (LoginTokens->status.code != WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to get login token, StatusCode: %d, Error: %s"), LoginTokens->status.code,
			   UTF8_TO_TCHAR(LoginTokens->status.detail));
		return;
	}

	if (LoginTokens->login_token_count == 0)
	{
		UE_LOG(LogSpatialWorkerConnection, Warning,
			   TEXT("No deployment found to connect to. Did you add the 'dev_login' tag to the deployment you want to connect to?"));
		return;
	}

	UE_LOG(LogSpatialWorkerConnection, Verbose, TEXT("Successfully received LoginTokens, Count: %d"), LoginTokens->login_token_count);
	USpatialConnectionManager* ConnectionManager = static_cast<USpatialConnectionManager*>(UserData);
	ConnectionManager->ProcessLoginTokensResponse(LoginTokens);
}

void USpatialConnectionManager::ProcessLoginTokensResponse(const Worker_LoginTokensResponse* LoginTokens)
{
	// If LoginTokenResCallback is callable and returns true, return early.
	if (LoginTokenResCallback && LoginTokenResCallback(LoginTokens))
	{
		return;
	}

	FString DeploymentToConnect = DevAuthConfig.Deployment;
	// If not set, use the first deployment. It can change every query if you have multiple items available, because the order is not
	// guaranteed.
	if (DeploymentToConnect.IsEmpty())
	{
		DevAuthConfig.LoginToken = FString(LoginTokens->login_tokens[0].login_token);
		DeploymentToConnect = UTF8_TO_TCHAR(LoginTokens->login_tokens[0].deployment_name);
	}
	else
	{
		bool bFoundDeployment = false;

		for (uint32 i = 0; i < LoginTokens->login_token_count; i++)
		{
			FString DeploymentName = UTF8_TO_TCHAR(LoginTokens->login_tokens[i].deployment_name);
			if (DeploymentToConnect.Compare(DeploymentName) == 0)
			{
				DevAuthConfig.LoginToken = FString(LoginTokens->login_tokens[i].login_token);
				bFoundDeployment = true;
				break;
			}
		}

		if (!bFoundDeployment)
		{
			OnConnectionFailure(WORKER_CONNECTION_STATUS_CODE_NETWORK_ERROR,
								FString::Printf(TEXT("Deployment not found! Make sure that the deployment with name '%s' is running and "
													 "has the 'dev_login' deployment tag."),
												*DeploymentToConnect));
			return;
		}
	}

	UE_LOG(LogSpatialConnectionManager, Log, TEXT("Dev auth flow: connecting to deployment \"%s\""), *DeploymentToConnect);
	ConnectToLocator(&DevAuthConfig);
}

void USpatialConnectionManager::RequestDeploymentLoginTokens()
{
	Worker_LoginTokensRequest LTParams{};
	FTCHARToUTF8 PlayerIdentityToken(*DevAuthConfig.PlayerIdentityToken);
	LTParams.player_identity_token = PlayerIdentityToken.Get();
	FTCHARToUTF8 WorkerType(*DevAuthConfig.WorkerType);
	LTParams.worker_type = WorkerType.Get();
	LTParams.use_insecure_connection = false;

	if (Worker_LoginTokensResponseFuture* LTFuture =
			Worker_CreateDevelopmentLoginTokensAsync(TCHAR_TO_UTF8(*DevAuthConfig.LocatorHost), DevAuthConfig.LocatorPort, &LTParams))
	{
		Worker_LoginTokensResponseFuture_Get(LTFuture, nullptr, this, &USpatialConnectionManager::OnLoginTokens);
	}
}

void USpatialConnectionManager::OnPlayerIdentityToken(void* UserData, const Worker_PlayerIdentityTokenResponse* PIToken)
{
	if (PIToken->status.code != WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to get PlayerIdentityToken, StatusCode: %d, Error: %s"),
			   PIToken->status.code, UTF8_TO_TCHAR(PIToken->status.detail));
		return;
	}

	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Successfully received PIToken: %s"), UTF8_TO_TCHAR(PIToken->player_identity_token));
	USpatialConnectionManager* ConnectionManager = static_cast<USpatialConnectionManager*>(UserData);
	ConnectionManager->DevAuthConfig.PlayerIdentityToken = UTF8_TO_TCHAR(PIToken->player_identity_token);

	ConnectionManager->RequestDeploymentLoginTokens();
}

void USpatialConnectionManager::StartDevelopmentAuth(const FString& DevAuthToken)
{
	FTCHARToUTF8 DAToken(*DevAuthToken);
	FTCHARToUTF8 PlayerId(*DevAuthConfig.PlayerId);
	FTCHARToUTF8 DisplayName(*DevAuthConfig.DisplayName);
	FTCHARToUTF8 MetaData(*DevAuthConfig.MetaData);

	Worker_PlayerIdentityTokenRequest PITParams{};
	PITParams.development_authentication_token = DAToken.Get();
	PITParams.player_id = PlayerId.Get();
	PITParams.display_name = DisplayName.Get();
	PITParams.metadata = MetaData.Get();
	PITParams.use_insecure_connection = false;

	if (Worker_PlayerIdentityTokenResponseFuture* PITFuture = Worker_CreateDevelopmentPlayerIdentityTokenAsync(
			TCHAR_TO_UTF8(*DevAuthConfig.LocatorHost), DevAuthConfig.LocatorPort, &PITParams))
	{
		Worker_PlayerIdentityTokenResponseFuture_Get(PITFuture, nullptr, this, &USpatialConnectionManager::OnPlayerIdentityToken);
	}
}

void USpatialConnectionManager::ConnectToReceptionist(uint32 PlayInEditorID)
{
	ReceptionistConfig.PreConnectInit(bConnectAsClient);

	ConfigureConnection ConnectionConfig(ReceptionistConfig, bConnectAsClient);

	TSharedPtr<SpatialGDK::SpatialEventTracer> EventTracer = CreateEventTracer(ReceptionistConfig.WorkerId);
	ConnectionConfig.Params.event_tracer = EventTracer != nullptr ? EventTracer->GetWorkerEventTracer() : nullptr;

	Worker_ConnectionFuture* ConnectionFuture =
		Worker_ConnectAsync(TCHAR_TO_UTF8(*ReceptionistConfig.GetReceptionistHost()), ReceptionistConfig.GetReceptionistPort(),
							TCHAR_TO_UTF8(*ReceptionistConfig.WorkerId), &ConnectionConfig.Params);

	FinishConnecting(ConnectionFuture, MoveTemp(EventTracer));
}

void USpatialConnectionManager::ConnectToLocator(FLocatorConfig* InLocatorConfig)
{
	if (InLocatorConfig == nullptr)
	{
		UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Trying to connect to locator with invalid locator config"));
		return;
	}

	InLocatorConfig->PreConnectInit(bConnectAsClient);

	ConfigureConnection ConnectionConfig(*InLocatorConfig, bConnectAsClient);

	TSharedPtr<SpatialGDK::SpatialEventTracer> EventTracer = CreateEventTracer(InLocatorConfig->WorkerId);
	ConnectionConfig.Params.event_tracer = EventTracer != nullptr ? EventTracer->GetWorkerEventTracer() : nullptr;

	FTCHARToUTF8 PlayerIdentityTokenCStr(*InLocatorConfig->PlayerIdentityToken);
	FTCHARToUTF8 LoginTokenCStr(*InLocatorConfig->LoginToken);

	Worker_LocatorParameters LocatorParams = {};
	LocatorParams.player_identity.player_identity_token = PlayerIdentityTokenCStr.Get();
	LocatorParams.player_identity.login_token = LoginTokenCStr.Get();

	// Connect to the locator on the default port(0 will choose the default)
	WorkerLocator = Worker_Locator_Create(TCHAR_TO_UTF8(*InLocatorConfig->LocatorHost), InLocatorConfig->LocatorPort, &LocatorParams);

	Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(WorkerLocator, &ConnectionConfig.Params);

	FinishConnecting(ConnectionFuture, MoveTemp(EventTracer));
}

void USpatialConnectionManager::FinishConnecting(Worker_ConnectionFuture* ConnectionFuture,
												 TSharedPtr<SpatialGDK::SpatialEventTracer> NewEventTracer)
{
	TWeakObjectPtr<USpatialConnectionManager> WeakSpatialConnectionManager(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, WeakSpatialConnectionManager,
															 EventTracing = MoveTemp(NewEventTracer)]() mutable {
		Worker_Connection* NewCAPIWorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);
		Worker_ConnectionFuture_Destroy(ConnectionFuture);

		AsyncTask(ENamedThreads::GameThread, [WeakSpatialConnectionManager, NewCAPIWorkerConnection,
											  EventTracing = MoveTemp(EventTracing)]() mutable {
			if (!WeakSpatialConnectionManager.IsValid())
			{
				// The game instance was destroyed before the connection finished, so just clean up the connection.
				Worker_Connection_Destroy(NewCAPIWorkerConnection);
				return;
			}

			USpatialConnectionManager* SpatialConnectionManager = WeakSpatialConnectionManager.Get();

			const uint8_t ConnectionStatusCode = Worker_Connection_GetConnectionStatusCode(NewCAPIWorkerConnection);

			if (ConnectionStatusCode == WORKER_CONNECTION_STATUS_CODE_SUCCESS)
			{
				const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
				SpatialConnectionManager->WorkerConnection = NewObject<USpatialWorkerConnection>();

				FComponentSetData SetData;

				for (Worker_ComponentId i = 0; i < 100; ++i)
				{
					SetData.ComponentSets.FindOrAdd(SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID).Add(i);
				}
				for (Worker_ComponentId i = 100; i < 100000; ++i)
				{
					SetData.ComponentSets.Add(static_cast<Worker_ComponentSetId>(i)).Add(i);
				}

				SpatialConnectionManager->WorkerConnection->SetConnection(NewCAPIWorkerConnection, MoveTemp(EventTracing),
																		  MoveTemp(SetData));
				SpatialConnectionManager->OnConnectionSuccess();
			}
			else
			{
				const FString ErrorMessage(UTF8_TO_TCHAR(Worker_Connection_GetConnectionStatusDetailString(NewCAPIWorkerConnection)));
				Worker_Connection_Destroy(NewCAPIWorkerConnection);
				SpatialConnectionManager->OnConnectionFailure(ConnectionStatusCode, ErrorMessage);
			}
		});
	});
}

ESpatialConnectionType USpatialConnectionManager::GetConnectionType() const
{
	return ConnectionType;
}

void USpatialConnectionManager::SetConnectionType(ESpatialConnectionType InConnectionType)
{
	// The locator config may not have been initialized
	check(!(InConnectionType == ESpatialConnectionType::Locator && LocatorConfig.LocatorHost.IsEmpty()));

	ConnectionType = InConnectionType;
}

bool USpatialConnectionManager::TrySetupConnectionConfigFromCommandLine(const FString& SpatialWorkerType)
{
	bool bSuccessfullyLoaded = LocatorConfig.TryLoadCommandLineArgs();
	if (bSuccessfullyLoaded)
	{
		UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Successfully set up locator config from command line arguments"));
		SetConnectionType(ESpatialConnectionType::Locator);
		LocatorConfig.WorkerType = SpatialWorkerType;
	}
	else
	{
		bSuccessfullyLoaded = DevAuthConfig.TryLoadCommandLineArgs();
		if (bSuccessfullyLoaded)
		{
			UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Successfully set up dev auth config from command line arguments"));
			SetConnectionType(ESpatialConnectionType::DevAuthFlow);
			DevAuthConfig.WorkerType = SpatialWorkerType;
		}
		else
		{
			UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Setting up receptionist config from command line arguments"));
			bSuccessfullyLoaded = ReceptionistConfig.TryLoadCommandLineArgs();
			if (bSuccessfullyLoaded)
			{
				ReceptionistConfig.WorkerType = SpatialWorkerType;
				SetConnectionType(ESpatialConnectionType::Receptionist);
			}
		}
	}

	return bSuccessfullyLoaded;
}

void USpatialConnectionManager::SetupConnectionConfigFromURL(const FURL& URL, const FString& SpatialWorkerType)
{
	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Setting up connection config from URL"));

	if (URL.HasOption(TEXT("locator")) || URL.HasOption(TEXT("devauth")))
	{
		FString LocatorHostOverride;
		if (URL.HasOption(TEXT("customLocator")))
		{
			LocatorHostOverride = URL.Host;
		}
		else
		{
			FParse::Value(FCommandLine::Get(), TEXT("locatorHost"), LocatorHostOverride);
		}

		int32 LocatorPortOverride = SpatialConstants::LOCATOR_PORT;
		if (const TCHAR* LocatorPortOption = URL.GetOption(TEXT("locatorPort="), nullptr))
		{
			LocatorPortOverride = FCString::Atoi(LocatorPortOption);
		}

		if (URL.HasOption(TEXT("devauth")))
		{
			// Use devauth login flow.
			SetConnectionType(ESpatialConnectionType::DevAuthFlow);
			if (LocatorHostOverride != "")
			{
				DevAuthConfig.LocatorHost = LocatorHostOverride;
			}
			DevAuthConfig.LocatorPort = LocatorPortOverride;
			DevAuthConfig.DevelopmentAuthToken = URL.GetOption(*SpatialConstants::URL_DEV_AUTH_TOKEN_OPTION, TEXT(""));
			DevAuthConfig.Deployment = URL.GetOption(*SpatialConstants::URL_TARGET_DEPLOYMENT_OPTION, TEXT(""));
			DevAuthConfig.PlayerId = URL.GetOption(*SpatialConstants::URL_PLAYER_ID_OPTION, *SpatialConstants::DEVELOPMENT_AUTH_PLAYER_ID);
			DevAuthConfig.DisplayName = URL.GetOption(*SpatialConstants::URL_DISPLAY_NAME_OPTION, TEXT(""));
			DevAuthConfig.MetaData = URL.GetOption(*SpatialConstants::URL_METADATA_OPTION, TEXT(""));
			DevAuthConfig.WorkerType = SpatialWorkerType;
		}
		else
		{
			// Use locator login flow.
			SetConnectionType(ESpatialConnectionType::Locator);
			if (LocatorHostOverride != "")
			{
				LocatorConfig.LocatorHost = LocatorHostOverride;
			}
			LocatorConfig.LocatorPort = LocatorPortOverride;
			LocatorConfig.PlayerIdentityToken = URL.GetOption(*SpatialConstants::URL_PLAYER_IDENTITY_OPTION, TEXT(""));
			LocatorConfig.LoginToken = URL.GetOption(*SpatialConstants::URL_LOGIN_OPTION, TEXT(""));
			LocatorConfig.WorkerType = SpatialWorkerType;
		}
	}
	else
	{
		SetConnectionType(ESpatialConnectionType::Receptionist);

		ReceptionistConfig.SetupFromURL(URL);
		ReceptionistConfig.WorkerType = SpatialWorkerType;
	}
}

void USpatialConnectionManager::OnConnectionSuccess()
{
	bIsConnected = true;

	OnConnectedCallback.ExecuteIfBound();
}

void USpatialConnectionManager::OnConnectionFailure(uint8_t ConnectionStatusCode, const FString& ErrorMessage)
{
	bIsConnected = false;

	OnFailedToConnectCallback.ExecuteIfBound(ConnectionStatusCode, ErrorMessage);
}

TSharedPtr<SpatialGDK::SpatialEventTracer> USpatialConnectionManager::CreateEventTracer(const FString& WorkerId)
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings == nullptr || !Settings->bEventTracingEnabled)
	{
		return nullptr;
	}

	return MakeShared<SpatialGDK::SpatialEventTracer>(WorkerId);
};
