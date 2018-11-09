// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

#include "Async/Async.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

void USpatialWorkerConnection::FinishDestroy()
{
	DestroyConnection();

	Super::FinishDestroy();
}

void USpatialWorkerConnection::DestroyConnection()
{
	if (WorkerConnection)
	{
		Worker_Connection_Destroy(WorkerConnection);
		WorkerConnection = nullptr;
	}

	if (WorkerLocator)
	{
		Worker_Locator_Destroy(WorkerLocator);
		WorkerLocator = nullptr;
	}
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
{
	if (bIsConnected)
	{
		OnConnected.ExecuteIfBound();
		return;
	}

	switch (GetConnectionType())
	{
	case SpatialConnectionType::Receptionist:
		ConnectToReceptionist(bInitAsClient);
		break;

	case SpatialConnectionType::Locator:
		ConnectToLocator();
		break;

	case SpatialConnectionType::LocatorV2:
		ConnectToLocatorV2();
		break;
	}
}

void USpatialWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bConnectAsClient ? SpatialConstants::ClientWorkerType : SpatialConstants::ServerWorkerType;
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

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = ReceptionistConfig.LinkProtocol;
	ConnectionParams.network.use_external_ip = ReceptionistConfig.UseExternalIp;

	FString ProtocolLogDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir()) + TEXT("protocol-log-");
#if PLATFORM_WINDOWS == 1 || PLATFORM_XBOXONE == 1
	ConnectionParams.enable_protocol_logging_at_startup = true;
#if PLATFORM_XBOXONE == 1
	ConnectionParams.protocol_logging.log_prefix = "T:\\protocol-log-";
#else
	ConnectionParams.protocol_logging.log_prefix = TCHAR_TO_UTF8(*ProtocolLogDir);
#endif
	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Protocol log prefix: %s"), UTF8_TO_TCHAR(ConnectionParams.protocol_logging.log_prefix));
#endif
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
			AsyncTask(ENamedThreads::GameThread, [this]
			{
				this->bIsConnected = true;
				this->OnConnected.ExecuteIfBound();
			});
		}
		else
		{
			GetAndPrintConnectionFailureMessage();
			// TODO: Try to reconnect - UNR-576
		}
	});
}

void USpatialWorkerConnection::ConnectToLocator()
{
	if (LocatorConfig.WorkerType.IsEmpty())
	{
		LocatorConfig.WorkerType = SpatialConstants::ClientWorkerType;
	}

	if (LocatorConfig.WorkerId.IsEmpty())
	{
		LocatorConfig.WorkerId = LocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 ProjectNameCStr(*LocatorConfig.ProjectName);
	FTCHARToUTF8 LoginTokenCStr(*LocatorConfig.LoginToken);

	Worker_LoginTokenCredentials Credentials;
	Credentials.token = LoginTokenCStr.Get();

	Worker_LocatorParameters LocatorParams = {};
	LocatorParams.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
	LocatorParams.project_name = ProjectNameCStr.Get();
	LocatorParams.login_token = Credentials;
	LocatorParams.enable_logging = true;

	WorkerLocator = Worker_Locator_Create(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), &LocatorParams);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(WorkerLocator);
	Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, this,
		[](void* UserData, const Worker_DeploymentList* DeploymentList)
	{
		USpatialWorkerConnection* SpatialConnection = static_cast<USpatialWorkerConnection*>(UserData);

		if (DeploymentList->error != nullptr)
		{
			const FString ErrorMessage = FString::Printf(TEXT("Error fetching deployment list: %s"), UTF8_TO_TCHAR(DeploymentList->error));
			UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to connect to SpatialOS: %s"), *ErrorMessage);
			SpatialConnection->OnConnectFailed.ExecuteIfBound(ErrorMessage);
			return;
		}

		if (DeploymentList->deployment_count == 0)
		{
			const FString ErrorMessage = FString::Printf(TEXT("Received empty list of deployments."));
			UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to connect to SpatialOS: %s"), *ErrorMessage);
			SpatialConnection->OnConnectFailed.ExecuteIfBound(ErrorMessage);
			return;
		}

		// TODO: Move creation of connection parameters into a function somehow
		Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
		FTCHARToUTF8 WorkerTypeCStr(*SpatialConnection->LocatorConfig.WorkerType);
		ConnectionParams.worker_type = WorkerTypeCStr.Get();
		ConnectionParams.enable_protocol_logging_at_startup = SpatialConnection->LocatorConfig.EnableProtocolLoggingAtStartup;

		Worker_ComponentVtable DefaultVtable = {};
		ConnectionParams.component_vtable_count = 0;
		ConnectionParams.default_component_vtable = &DefaultVtable;

		ConnectionParams.network.connection_type = SpatialConnection->LocatorConfig.LinkProtocol;
		ConnectionParams.network.use_external_ip = SpatialConnection->LocatorConfig.UseExternalIp;
		// end TODO

		int DeploymentIndex = 0;
		if (!SpatialConnection->LocatorConfig.DeploymentName.IsEmpty())
		{
			bool bFoundRequestedDeployment = false;
			for (uint32_t i = 0; i < DeploymentList->deployment_count; ++i)
			{
				if (SpatialConnection->LocatorConfig.DeploymentName.Equals(UTF8_TO_TCHAR(DeploymentList->deployments[i].deployment_name)))
				{
					DeploymentIndex = i;
					bFoundRequestedDeployment = true;
					break;
				}
			}

			if (!bFoundRequestedDeployment)
			{
				const FString ErrorMessage = FString::Printf(TEXT("Requested deployment name was not present in the deployment list: %s"),
					*SpatialConnection->LocatorConfig.DeploymentName);
				UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to connect to SpatialOS: %s"), *ErrorMessage);
				SpatialConnection->OnConnectFailed.ExecuteIfBound(ErrorMessage);
				return;
			}
		}

		Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(SpatialConnection->WorkerLocator, DeploymentList->deployments[DeploymentIndex].deployment_name,
				&ConnectionParams, nullptr, nullptr);

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, SpatialConnection]
		{
			SpatialConnection->WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

			Worker_ConnectionFuture_Destroy(ConnectionFuture);
			if (Worker_Connection_IsConnected(SpatialConnection->WorkerConnection))
			{
				AsyncTask(ENamedThreads::GameThread, [SpatialConnection]
				{
					SpatialConnection->bIsConnected = true;
					SpatialConnection->OnConnected.ExecuteIfBound();
				});
			}
			else
			{
				SpatialConnection->GetAndPrintConnectionFailureMessage();
			}
		});
	});
}


void USpatialWorkerConnection::ConnectToLocatorV2()
{
	if (LocatorV2Config.WorkerType.IsEmpty())
	{
		LocatorV2Config.WorkerType = SpatialConstants::ClientWorkerType;
	}

	if (LocatorV2Config.WorkerId.IsEmpty())
	{
		LocatorV2Config.WorkerId = LocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 PlayerIdentityTokenCStr(*LocatorV2Config.PlayerIdentityToken);
	FTCHARToUTF8 LoginTokenCStr(*LocatorV2Config.LoginToken);

	Worker_Alpha_LocatorParameters LocatorParams = {};
	LocatorParams.player_identity.player_identity_token = PlayerIdentityTokenCStr.Get();
	LocatorParams.player_identity.login_token = LoginTokenCStr.Get();

	WorkerLocatorV2 = Worker_Alpha_Locator_Create(TCHAR_TO_UTF8(*LocatorV2Config.LocatorHost), 0, &LocatorParams);

	// TODO: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*LocatorV2Config.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = LocatorV2Config.EnableProtocolLoggingAtStartup;

	Worker_ComponentVtable DefaultVtable = {};
	ConnectionParams.component_vtable_count = 0;
	ConnectionParams.default_component_vtable = &DefaultVtable;

	ConnectionParams.network.connection_type = LocatorV2Config.LinkProtocol;
	ConnectionParams.network.use_external_ip = LocatorV2Config.UseExternalIp;

	FString ProtocolLogDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir()) + TEXT("protocol-log-");
#if PLATFORM_WINDOWS == 1 || PLATFORM_XBOXONE == 1
	ConnectionParams.enable_protocol_logging_at_startup = true;
#if PLATFORM_XBOXONE == 1
	ConnectionParams.protocol_logging.log_prefix = "T:\\protocol-log-";
#else
	ConnectionParams.protocol_logging.log_prefix = TCHAR_TO_UTF8(*ProtocolLogDir);
#endif
	UE_LOG(LogSpatialWorkerConnection, Log, TEXT("Protocol log prefix: %s"), UTF8_TO_TCHAR(ConnectionParams.protocol_logging.log_prefix));
#endif
	// end TODO

	Worker_ConnectionFuture* ConnectionFuture = Worker_Alpha_Locator_ConnectAsync(WorkerLocatorV2, &ConnectionParams);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, this]
	{
		WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

		Worker_ConnectionFuture_Destroy(ConnectionFuture);
		if (Worker_Connection_IsConnected(WorkerConnection))
		{
			AsyncTask(ENamedThreads::GameThread, [this]
			{
				this->bIsConnected = true;
				this->OnConnected.ExecuteIfBound();
			});
		}
		else
		{
			Worker_OpList* OpList = Worker_Connection_GetOpList(WorkerConnection, 0);
			for (int i = 0; i < (int)OpList->op_count; i++)
			{
				if (OpList->ops[i].op_type == WORKER_OP_TYPE_DISCONNECT)
				{
					UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Couldn't connect to SpatialOS: %s"), UTF8_TO_TCHAR(OpList->ops[i].disconnect.reason));
					GEngine->AddOnScreenDebugMessage(-1, 200.0f, FColor::Red, FString::Printf(TEXT("Couldn't connect to SpatialOS: %s"), UTF8_TO_TCHAR(OpList->ops[i].disconnect.reason)));
				}
			}

			// TODO: Try to reconnect - UNR-576
		}
	});
}

SpatialConnectionType USpatialWorkerConnection::GetConnectionType() const
{
	if (!LocatorV2Config.LoginToken.IsEmpty())
	{
		return SpatialConnectionType::LocatorV2;
	}
	else if (!LocatorConfig.LoginToken.IsEmpty())
	{
		return SpatialConnectionType::Locator;
	}
	else
	{
		return SpatialConnectionType::Receptionist;
	}
}

void USpatialWorkerConnection::GetAndPrintConnectionFailureMessage()
{
	Worker_OpList* OpList = Worker_Connection_GetOpList(WorkerConnection, 0);
	for (int i = 0; i < static_cast<int>(OpList->op_count); i++)
	{
		if (OpList->ops[i].op_type == WORKER_OP_TYPE_DISCONNECT)
		{
			const FString ErrorMessage(UTF8_TO_TCHAR(OpList->ops[i].disconnect.reason));
			AsyncTask(ENamedThreads::GameThread, [this, ErrorMessage]
			{
				UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Failed to connect to SpatialOS: %s"), *ErrorMessage);
				OnConnectFailed.ExecuteIfBound(ErrorMessage);
			});
			break;
		}
	}
}

Worker_OpList* USpatialWorkerConnection::GetOpList()
{
	return Worker_Connection_GetOpList(WorkerConnection, 0);
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdRequest()
{
	return Worker_Connection_SendReserveEntityIdRequest(WorkerConnection, nullptr);
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
	Worker_Connection_SendComponentUpdate(WorkerConnection, EntityId, ComponentUpdate);
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	Worker_CommandParameters CommandParams{};
	return Worker_Connection_SendCommandRequest(WorkerConnection, EntityId, Request, CommandId, nullptr, &CommandParams);
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	return Worker_Connection_SendCommandResponse(WorkerConnection, RequestId, Response);
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

FString USpatialWorkerConnection::GetWorkerId() const
{
	return FString(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(WorkerConnection)));
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntiyQuery)
{
	return Worker_Connection_SendEntityQueryRequest(WorkerConnection, EntiyQuery, 0);
}
