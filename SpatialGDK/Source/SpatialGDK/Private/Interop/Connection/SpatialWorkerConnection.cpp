// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Utils/ErrorCodeRemapping.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Engine/World.h"
#include "UnrealEngine.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "SNotificationList.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

//TArray<FString> USpatialWorkerConnection::WorkerIds;

void USpatialWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	if (WorkerConnection)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerConnection = WorkerConnection]
		{
			Worker_Connection_Destroy(WorkerConnection);
		});

		WorkerConnection = nullptr;
	}

	if (WorkerLegacyLocator)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerLegacyLocator = WorkerLegacyLocator]
		{
			Worker_Locator_Destroy(WorkerLegacyLocator);
		});

		WorkerLegacyLocator = nullptr;
	}

	if (WorkerLocator)
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WorkerLocator = WorkerLocator]
		{
			Worker_Alpha_Locator_Destroy(WorkerLocator);
		});

		WorkerLocator = nullptr;
	}

	bIsConnected = false;
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
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

void USpatialWorkerConnection::ConnectToReceptionist(bool bInConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bInConnectAsClient ? SpatialConstants::ClientWorkerType : SpatialConstants::ServerWorkerType;
		UE_LOG(LogSpatialWorkerConnection, Warning, TEXT("No worker type specified through commandline, defaulting to %s"), *ReceptionistConfig.WorkerType);
	}

	if (ReceptionistConfig.WorkerId.IsEmpty())
	{
		ReceptionistConfig.WorkerId = ReceptionistConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	bConnectAsClient = bInConnectAsClient;
	PlayInEditorID = GPlayInEditorID;

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

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

		Worker_ConnectionFuture_Destroy(ConnectionFuture);
		if (Worker_Connection_IsConnected(WorkerConnection))
		{
			ReplaceWorker();
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
	});
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

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
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
	});
}

SpatialConnectionType USpatialWorkerConnection::GetConnectionType() const
{
	// The legacy locator path did not specify PlayerIdentityToken, so if we have one
	// we can use the new locator workflow.
	if (!LocatorConfig.PlayerIdentityToken.IsEmpty())
	{
		return SpatialConnectionType::Locator;
	}
	else if (!LegacyLocatorConfig.LoginToken.IsEmpty())
	{
		return SpatialConnectionType::LegacyLocator;
	}
	else
	{
		return SpatialConnectionType::Receptionist;
	}
}

void USpatialWorkerConnection::ClearWorkerIds()
{
	//WorkerIds.Empty();
}

Worker_OpList* USpatialWorkerConnection::GetOpList()
{
	return Worker_Connection_GetOpList(WorkerConnection, 0);
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return Worker_Connection_SendReserveEntityIdsRequest(WorkerConnection, NumOfEntities, nullptr);
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(uint32_t ComponentCount, const Worker_ComponentData* Components, const Worker_EntityId* EntityId)
{
	return Worker_Connection_SendCreateEntityRequest(WorkerConnection, ComponentCount, Components, EntityId, nullptr);
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return Worker_Connection_SendDeleteEntityRequest(WorkerConnection, EntityId, nullptr);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate)
{
	Worker_Alpha_UpdateParameters UpdateParameters{};
	UpdateParameters.loopback = false;
	Worker_Alpha_Connection_SendComponentUpdate(WorkerConnection, EntityId, ComponentUpdate, &UpdateParameters);
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	Worker_CommandParameters CommandParams{};
	return Worker_Connection_SendCommandRequest(WorkerConnection, EntityId, Request, CommandId, nullptr, &CommandParams);
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	Worker_Connection_SendCommandResponse(WorkerConnection, RequestId, Response);
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const char* Message)
{
	Worker_Connection_SendCommandFailure(WorkerConnection, RequestId, Message);
}

void USpatialWorkerConnection::SendLogMessage(const uint8_t Level, const char* LoggerName, const char* Message)
{
	Worker_LogMessage LogMessage{};
	LogMessage.level = Level;
	LogMessage.logger_name = LoggerName;
	LogMessage.message = Message;

	Worker_Connection_SendLogMessage(WorkerConnection, &LogMessage);
}

void USpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, const TArray<Worker_InterestOverride>& ComponentInterest)
{
	Worker_Connection_SendComponentInterest(WorkerConnection, EntityId, ComponentInterest.GetData(), ComponentInterest.Num());
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return Worker_Connection_SendEntityQueryRequest(WorkerConnection, EntityQuery, 0);
}

void USpatialWorkerConnection::SendMetrics(const Worker_Metrics* Metrics)
{
	Worker_Connection_SendMetrics(WorkerConnection, Metrics);
}

FString USpatialWorkerConnection::GetWorkerId() const
{
	return FString(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(WorkerConnection)));
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	return CachedWorkerAttributes;
}

void USpatialWorkerConnection::CacheWorkerAttributes()
{
	const Worker_WorkerAttributes* Attributes = Worker_Connection_GetWorkerAttributes(WorkerConnection);

	CachedWorkerAttributes.Empty();
	for (uint32 Index = 0; Index < Attributes->attribute_count; ++Index)
	{
		CachedWorkerAttributes.Add(UTF8_TO_TCHAR(Attributes->attributes[Index]));
	}
}

void USpatialWorkerConnection::ReplaceWorker()
{
	static TArray<FString> WorkerIds;

	if (!bConnectAsClient)
	{
		FString OldWorkerId;

		if (PlayInEditorID <= WorkerIds.Num())
		{
			OldWorkerId = WorkerIds[PlayInEditorID - 1];
			//OldWorkerId = FString::Printf(TEXT("%s:%d"), *ReceptionistConfig.WorkerType, PlayInEditorID - 1);

			//const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

			//const FString ExecuteAbsolutePath = SpatialGDKSettings->GetSpatialOSDirectory();
			const FString ExecuteAbsolutePath(TEXT("C:/dev/UnrealGDKTestSuite/spatial/"));
			const FString CmdExecutable = TEXT("cmd.exe");

			const FString SpatialCmdArgument = FString::Printf(
				//TEXT("/c spatial.exe local worker replace "
				TEXT("/c cmd.exe /c spatial.exe local worker replace "
					"--existing_worker_id %s "
					"--local_service_grpc_port 22000 "
					"--replacing_worker_id %s "
					"--log_level=debug ^& pause"), *ReceptionistConfig.WorkerId, *OldWorkerId);

			//UE_LOG(LogSpatialGDKEditorToolbar, Log, TEXT("Starting cmd.exe with `%s` arguments."), *SpatialCmdArgument);
			// Temporary workaround: To get spatial.exe to properly show a window we have to call cmd.exe to
			// execute it. We currently can't use pipes to capture output as it doesn't work properly with current
			// spatial.exe.
			uint32 SpatialOSStackProcessID = 0;
			FProcHandle SpatialOSStackProcHandle = FPlatformProcess::CreateProc(
				*(CmdExecutable), *SpatialCmdArgument, true, false, false, &SpatialOSStackProcessID, 0,
				*ExecuteAbsolutePath, nullptr, nullptr);
			//  
			//  			FNotificationInfo Info(SpatialOSStackProcHandle.IsValid() == true
			//  				? FText::FromString(TEXT("SpatialOS Starting..."))
			//  				: FText::FromString(TEXT("Failed to start SpatialOS")));
		}
		else
		{
			//ReceptionistConfig.WorkerId = FString::Printf(TEXT("%s:%d"), *ReceptionistConfig.WorkerType, PlayInEditorID - 1);
			WorkerIds.AddDefaulted();
			WorkerIds[PlayInEditorID - 1] = ReceptionistConfig.WorkerId;
		}
	}
}

USpatialNetDriver* USpatialWorkerConnection::GetSpatialNetDriverChecked() const
{
	UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter());
	UNetDriver* NetDriver = GameInstance->GetWorld()->GetNetDriver();

	// On the client, the world might not be completely set up.
	// in this case we can use the PendingNetGame to get the NetDriver
	if (NetDriver == nullptr)
	{
		NetDriver = GameInstance->GetWorldContext()->PendingNetGame->GetNetDriver();
	}

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(NetDriver);
	checkf(SpatialNetDriver, TEXT("SpatialNetDriver was invalid while accessing SpatialNetDriver!"));
	return SpatialNetDriver;
}

void USpatialWorkerConnection::OnConnectionSuccess()
{
	bIsConnected = true;
	GetSpatialNetDriverChecked()->HandleOnConnected();
}

void USpatialWorkerConnection::OnPreConnectionFailure(const FString& Reason)
{
	bIsConnected = false;
	GetSpatialNetDriverChecked()->HandleOnConnectionFailed(Reason);
}

void USpatialWorkerConnection::OnConnectionFailure()
{
	bIsConnected = false;

	UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter());
	if (GEngine != nullptr && GameInstance->GetWorld() != nullptr)
	{
		uint8_t ConnectionStatusCode = Worker_Connection_GetConnectionStatusCode(WorkerConnection);
		const FString ErrorMessage(UTF8_TO_TCHAR(Worker_Connection_GetConnectionStatusDetailString(WorkerConnection)));

		GEngine->BroadcastNetworkFailure(GameInstance->GetWorld(), GetSpatialNetDriverChecked(), ENetworkFailure::FromDisconnectOpStatusCode(ConnectionStatusCode), *ErrorMessage);
	}
}
