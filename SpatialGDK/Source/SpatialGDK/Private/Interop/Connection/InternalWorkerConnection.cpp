// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

DEFINE_LOG_CATEGORY(LogInternalWorkerConnection);

void USpatialWorkerConnection::ConnectToSpatialOS()
{
	if (bIsConnected)
	{
		OnConnectionSuccess();
		return;
	}

	switch (GetConnectionType())
	{
	case SpatialConnectionType::Receptionist:
		ConnectToReceptionist(bInitAsClient);
		break;
	case SpatialConnectionType::LegacyLocator:
		ConnectToLegacyLocator();
		break;
	case SpatialConnectionType::Locator:
		ConnectToLocator();
		break;
	}
}

Worker_ConnectionParameters UInternalWorkerConnection::CreateConnectionParams()
{

}

void UInternalWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bConnectAsClient ? SpatialConstants::ClientWorkerType : SpatialConstants::ServerWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *ReceptionistConfig.WorkerType);
	}

	if (ReceptionistConfig.WorkerId.IsEmpty())
	{
		ReceptionistConfig.WorkerId = ReceptionistConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	// TODO: Move creation of connection parameters into a function somehow - UNR:579
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*ReceptionistConfig.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = ReceptionistConfig.EnableProtocolLoggingAtStartup;

	FString FinalProtocolLoggingPrefix;
	if (!ReceptionistConfig.ProtocolLoggingPrefix.IsEmpty())
	{
		FinalProtocolLoggingPrefix = ReceptionistConfig.ProtocolLoggingPrefix;
	}
	else
	{
		FinalProtocolLoggingPrefix = ReceptionistConfig.WorkerId;
	}
	FTCHARToUTF8 ProtocolLoggingPrefixCStr(*FinalProtocolLoggingPrefix);
	ConnectionParams.protocol_logging.log_prefix = ProtocolLoggingPrefixCStr.Get();

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = ReceptionistConfig.LinkProtocol;
	ConnectionParams.network.use_external_ip = ReceptionistConfig.UseExternalIp;
	ConnectionParams.network.tcp.multiplex_level = ReceptionistConfig.TcpMultiplexLevel;
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_ConnectAsync(
		TCHAR_TO_UTF8(*ReceptionistConfig.ReceptionistHost), ReceptionistConfig.ReceptionistPort,
		TCHAR_TO_UTF8(*ReceptionistConfig.WorkerId), &ConnectionParams);

	WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

	Worker_ConnectionFuture_Destroy(ConnectionFuture);
	if (Worker_Connection_IsConnected(WorkerConnection))
	{
		CacheWorkerAttributes();

		AsyncTask(ENamedThreads::GameThread, [this]
		{
			this->OnConnectionSuccess();
		});
	}
	else
	{
		// TODO: Try to reconnect - UNR-576
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			this->OnConnectionFailure();
		});
	}
}

void USpatialWorkerConnection::ConnectToLegacyLocator()
{
	if (LegacyLocatorConfig.WorkerType.IsEmpty())
	{
		LegacyLocatorConfig.WorkerType = SpatialConstants::ClientWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *LegacyLocatorConfig.WorkerType);
	}

	if (LegacyLocatorConfig.WorkerId.IsEmpty())
	{
		LegacyLocatorConfig.WorkerId = LegacyLocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 ProjectNameCStr(*LegacyLocatorConfig.ProjectName);
	FTCHARToUTF8 LoginTokenCStr(*LegacyLocatorConfig.LoginToken);

	Worker_LoginTokenCredentials Credentials;
	Credentials.token = LoginTokenCStr.Get();

	Worker_LocatorParameters LocatorParams = {};
	LocatorParams.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
	LocatorParams.project_name = ProjectNameCStr.Get();
	LocatorParams.login_token = Credentials;

	WorkerLegacyLocator = Worker_Locator_Create(TCHAR_TO_UTF8(*LegacyLocatorConfig.LocatorHost), &LocatorParams);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(WorkerLegacyLocator);
	Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, this,
		[](void* UserData, const Worker_DeploymentList* DeploymentList)
	{
		USpatialWorkerConnection* SpatialConnection = static_cast<USpatialWorkerConnection*>(UserData);

		if (DeploymentList->error != nullptr)
		{
			const FString ErrorMessage = FString::Printf(TEXT("Error fetching deployment list: %s"), UTF8_TO_TCHAR(DeploymentList->error));
			SpatialConnection->OnPreConnectionFailure(ErrorMessage);
			return;
		}

		if (DeploymentList->deployment_count == 0)
		{
			const FString ErrorMessage = FString::Printf(TEXT("Received empty list of deployments."));
			SpatialConnection->OnPreConnectionFailure(ErrorMessage);
			return;
		}

		// TODO: Move creation of connection parameters into a function somehow
		Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
		FTCHARToUTF8 WorkerTypeCStr(*SpatialConnection->LegacyLocatorConfig.WorkerType);
		ConnectionParams.worker_type = WorkerTypeCStr.Get();
		ConnectionParams.enable_protocol_logging_at_startup = SpatialConnection->LegacyLocatorConfig.EnableProtocolLoggingAtStartup;

		Worker_ComponentVtable DefaultVtable = {};
		ConnectionParams.component_vtable_count = 0;
		ConnectionParams.default_component_vtable = &DefaultVtable;

		ConnectionParams.network.connection_type = SpatialConnection->LegacyLocatorConfig.LinkProtocol;
		ConnectionParams.network.use_external_ip = SpatialConnection->LegacyLocatorConfig.UseExternalIp;
		ConnectionParams.network.tcp.multiplex_level = SpatialConnection->LegacyLocatorConfig.TcpMultiplexLevel;
		// end TODO

		int DeploymentIndex = 0;
		if (!SpatialConnection->LegacyLocatorConfig.DeploymentName.IsEmpty())
		{
			bool bFoundRequestedDeployment = false;
			for (uint32_t i = 0; i < DeploymentList->deployment_count; ++i)
			{
				if (SpatialConnection->LegacyLocatorConfig.DeploymentName.Equals(UTF8_TO_TCHAR(DeploymentList->deployments[i].deployment_name)))
				{
					DeploymentIndex = i;
					bFoundRequestedDeployment = true;
					break;
				}
			}

			if (!bFoundRequestedDeployment)
			{
				const FString ErrorMessage = FString::Printf(TEXT("Requested deployment name was not present in the deployment list: %s"),
					*SpatialConnection->LegacyLocatorConfig.DeploymentName);
				SpatialConnection->OnPreConnectionFailure(ErrorMessage);
				return;
			}
		}

		Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(SpatialConnection->WorkerLegacyLocator, DeploymentList->deployments[DeploymentIndex].deployment_name,
			&ConnectionParams, nullptr, nullptr);

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, SpatialConnection]
		{
			SpatialConnection->WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

			Worker_ConnectionFuture_Destroy(ConnectionFuture);
			if (Worker_Connection_IsConnected(SpatialConnection->WorkerConnection))
			{
				SpatialConnection->CacheWorkerAttributes();

				AsyncTask(ENamedThreads::GameThread, [SpatialConnection]
				{
					SpatialConnection->OnConnectionSuccess();
				});
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, [SpatialConnection]
				{
					SpatialConnection->OnConnectionFailure();
				});
			}
		});
	});
}

void USpatialWorkerConnection::ConnectToLocator()
{
	if (LocatorConfig.WorkerType.IsEmpty())
	{
		LocatorConfig.WorkerType = SpatialConstants::ClientWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *LocatorConfig.WorkerType);
	}

	if (LocatorConfig.WorkerId.IsEmpty())
	{
		LocatorConfig.WorkerId = LocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 PlayerIdentityTokenCStr(*LocatorConfig.PlayerIdentityToken);
	FTCHARToUTF8 LoginTokenCStr(*LocatorConfig.LoginToken);

	Worker_Alpha_LocatorParameters LocatorParams = {};
	LocatorParams.player_identity.player_identity_token = PlayerIdentityTokenCStr.Get();
	LocatorParams.player_identity.login_token = LoginTokenCStr.Get();

	// Connect to the locator on the default port(0 will choose the default)
	WorkerLocator = Worker_Alpha_Locator_Create(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), 0, &LocatorParams);

	// TODO: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*LocatorConfig.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = LocatorConfig.EnableProtocolLoggingAtStartup;

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = LocatorConfig.LinkProtocol;
	ConnectionParams.network.use_external_ip = LocatorConfig.UseExternalIp;
	ConnectionParams.network.tcp.multiplex_level = LocatorConfig.TcpMultiplexLevel;

	FString ProtocolLogDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir()) + TEXT("protocol-log-");
	ConnectionParams.protocol_logging.log_prefix = TCHAR_TO_UTF8(*ProtocolLogDir);
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_Alpha_Locator_ConnectAsync(WorkerLocator, &ConnectionParams);

	WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

	Worker_ConnectionFuture_Destroy(ConnectionFuture);
	if (Worker_Connection_IsConnected(WorkerConnection))
	{
		CacheWorkerAttributes();
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			this->OnConnectionSuccess();
		});
	}
	else
	{
		// TODO: Try to reconnect - UNR-576
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			this->OnConnectionFailure();
		});
	}
}
