// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialWorkerConnection.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Engine/World.h"
#include "UnrealEngine.h"
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

USpatialNetDriver* USpatialWorkerConnection::GetSpatialNetDriverChecked() const
{
	check(GEngine);
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GEngine->GetWorldFromContextObjectChecked(this)->GetNetDriver());
	checkf(NetDriver, TEXT("SpatialNetDriver was invalid while accessing SpatialNetDriver!"));
	return NetDriver;
}


// TODO: UNR-962 - move connection events in to native connection handling codepath (eg, UEngine::HandleNetworkFailure)
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

	Worker_OpList* OpList = Worker_Connection_GetOpList(WorkerConnection, 0);
	for (size_t i = 0; i < OpList->op_count; i++)
	{
		if (OpList->ops[i].op_type == WORKER_OP_TYPE_DISCONNECT)
		{
			const FString ErrorMessage(UTF8_TO_TCHAR(OpList->ops[i].disconnect.reason));
			AsyncTask(ENamedThreads::GameThread, [this, ErrorMessage]
			{
				GetSpatialNetDriverChecked()->HandleOnConnectionFailed(ErrorMessage);
			});
			break;
		}
	}
}
