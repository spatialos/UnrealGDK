// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialWorkerConnection.h"

#include "Async/Async.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

void USpatialWorkerConnection::FinishDestroy()
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

	Super::FinishDestroy();
}

void USpatialWorkerConnection::Connect(bool bInitAsClient)
{
	if (ShouldConnectWithLocator())
	{
		ConnectToLocator();
	}
	else
	{
		ConnectToReceptionist(bInitAsClient);
	}

}

void USpatialWorkerConnection::ConnectToReceptionist(bool bConnectAsClient)
{
	if (ReceptionistConfig.WorkerType.IsEmpty())
	{
		ReceptionistConfig.WorkerType = bConnectAsClient ? TEXT("UnrealClient") : TEXT("UnrealWorker");
	}

	if (ReceptionistConfig.WorkerId.IsEmpty())
	{
		ReceptionistConfig.WorkerId = ReceptionistConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	// TODO: Move creation of connection parameters into a function somehow
	Worker_ConnectionParameters ConnectionParams = Worker_DefaultConnectionParameters();
	FTCHARToUTF8 WorkerTypeCStr(*ReceptionistConfig.WorkerType);
	ConnectionParams.worker_type = WorkerTypeCStr.Get();
	ConnectionParams.enable_protocol_logging_at_startup = ReceptionistConfig.EnableProtocolLoggingAtStartup;

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
				}
			}

			// TODO: try to reconnect
		}
	});
}

void USpatialWorkerConnection::ConnectToLocator()
{
	if (LocatorConfig.WorkerType.IsEmpty())
	{
		LocatorConfig.WorkerType = TEXT("UnrealClient");
	}

	if (LocatorConfig.WorkerId.IsEmpty())
	{
		LocatorConfig.WorkerId = LocatorConfig.WorkerType + FGuid::NewGuid().ToString();
	}

	FTCHARToUTF8 ProjectNameCStr(*LocatorConfig.ProjectName);
	FTCHARToUTF8 LoginTokenCStr(*LocatorConfig.LoginToken);

	Worker_LoginTokenCredentials Credentials;
	Credentials.token = LoginTokenCStr.Get();

	Worker_LocatorParameters LocatorParams;
	LocatorParams.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
	LocatorParams.project_name = ProjectNameCStr.Get();
	LocatorParams.login_token = Credentials;

	WorkerLocator = Worker_Locator_Create(TCHAR_TO_UTF8(*LocatorConfig.LocatorHost), &LocatorParams);

	Worker_DeploymentListFuture* DeploymentListFuture = Worker_Locator_GetDeploymentListAsync(WorkerLocator);
	Worker_DeploymentListFuture_Get(DeploymentListFuture, nullptr, this,
		[](void* UserData, const Worker_DeploymentList* DeploymentList)
	{
		if (DeploymentList->error != nullptr)
		{
			UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Error fetching deployment list: %s"), DeploymentList->error);
		}

		if (DeploymentList->deployment_count == 0)
		{
			UE_LOG(LogSpatialWorkerConnection, Error, TEXT("Received empty list of deployments. Failed to connect."));
		}

		USpatialWorkerConnection* SpatialConnection = static_cast<USpatialWorkerConnection*>(UserData);

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

		Worker_ConnectionFuture* ConnectionFuture = Worker_Locator_ConnectAsync(SpatialConnection->WorkerLocator, DeploymentList->deployments[0].deployment_name,
			&ConnectionParams, nullptr, nullptr);

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [ConnectionFuture, SpatialConnection]
		{
			SpatialConnection->WorkerConnection = Worker_ConnectionFuture_Get(ConnectionFuture, nullptr);

			Worker_ConnectionFuture_Destroy(ConnectionFuture);

			AsyncTask(ENamedThreads::GameThread, [SpatialConnection]
			{
				SpatialConnection->bIsConnected = true;
				SpatialConnection->OnConnected.ExecuteIfBound();
			});
		});
	});
}

bool USpatialWorkerConnection::ShouldConnectWithLocator()
{
	const TCHAR* CommandLine = FCommandLine::Get();
	FString TempStr;
	return FParse::Value(CommandLine, *FString("loginToken"), TempStr);
}

bool USpatialWorkerConnection::IsConnected()
{
	return bIsConnected;
}

Worker_OpList* USpatialWorkerConnection::GetOpList()
{
	return Worker_Connection_GetOpList(WorkerConnection, 0);
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdRequest()
{
	return Worker_Connection_SendReserveEntityIdRequest(WorkerConnection, nullptr);
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
